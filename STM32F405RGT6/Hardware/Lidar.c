#include "Lidar.h"
#include "usart.h"

/* N10 串口协议：帧头 A5 5A，固定 58 字节，每帧包含 16 个测距点。 */
#define LIDAR_FRAME_HEADER_0             0xA5U
#define LIDAR_FRAME_HEADER_1             0x5AU
#define LIDAR_FRAME_LENGTH               58U
#define LIDAR_POINTS_PER_FRAME           16U
#define LIDAR_MAX_SCAN_POINTS            450U

/* 小车识别参数：只保留 50～60 cm 距离带内的点。 */
#define LIDAR_CAR_DISTANCE_MIN_MM        500U
#define LIDAR_CAR_DISTANCE_MAX_MM        600U
#define LIDAR_NORMAL_POINT_GAP_CDEG      200U
#define LIDAR_OCCLUSION_GAP_CDEG         1000U
#define LIDAR_OCCLUSION_DISTANCE_GAP_MM  40U
#define LIDAR_MIN_CLUSTER_POINTS         3U
#define LIDAR_NO_BUFFER                   0xFFU

/* 单个雷达点，角度单位为 0.01°，距离单位为 mm。 */
typedef struct
{
    uint16_t angle_cdeg;
    uint16_t distance_mm;
} Lidar_Point;

/* 一整圈内落入目标距离带的点集合。 */
typedef struct
{
    Lidar_Point points[LIDAR_MAX_SCAN_POINTS];
    uint16_t count;
} Lidar_Scan;

/* 对外发布的识别结果；car_angle_valid 为 1 时数据有效。 */
volatile float car_angle = 0.0f;
volatile uint8_t car_angle_valid = 0U;
volatile uint16_t car_distance_mm = 0U;
volatile uint16_t lidar_target_point_count = 0U;
volatile uint32_t lidar_valid_frame_count = 0U;
volatile uint32_t lidar_error_frame_count = 0U;

static uint8_t lidar_rx_byte;
static uint8_t lidar_frame[LIDAR_FRAME_LENGTH];
static uint8_t lidar_frame_index;

/* 双缓冲用于隔离串口中断写入与主循环数据处理。 */
static Lidar_Scan lidar_scan[2];
static volatile uint8_t lidar_write_buffer;
static volatile uint8_t lidar_ready_buffer = LIDAR_NO_BUFFER;
static volatile uint8_t lidar_processing_buffer = LIDAR_NO_BUFFER;
static uint16_t lidar_last_angle_cdeg;
static uint8_t lidar_have_last_angle;

/**
  * @brief  从雷达协议数据中读取一个大端 16 位无符号整数。
  * @param  data 两字节数据的首地址，高字节在前。
  * @retval 解析得到的 16 位无符号整数。
  */
static uint16_t Lidar_ReadBE16(const uint8_t *data)
{
    return (uint16_t)(((uint16_t)data[0] << 8) | data[1]);
}

/**
  * @brief  计算两个 16 位无符号整数的绝对差。
  * @param  a 第一个数。
  * @param  b 第二个数。
  * @retval a 与 b 的绝对差。
  */
static uint16_t Lidar_AbsDiffU16(uint16_t a, uint16_t b)
{
    return (a >= b) ? (uint16_t)(a - b) : (uint16_t)(b - a);
}

/**
  * @brief  将当前写缓冲区中的一整圈目标点提交给主循环处理。
  * @note   本函数在串口接收中断中调用。若主循环尚未处理上一圈，则丢弃当前圈，
  *         防止覆盖待读取或正在处理的数据。
  */
static void Lidar_CommitScan(void)
{
    uint8_t next_buffer;

    next_buffer = (uint8_t)(lidar_write_buffer ^ 1U);

    /* 主循环正在处理的扫描缓冲区不得被中断接收过程覆盖。 */
    if ((lidar_ready_buffer == LIDAR_NO_BUFFER) &&
        (lidar_processing_buffer != next_buffer))
    {
        lidar_ready_buffer = lidar_write_buffer;
        lidar_write_buffer = next_buffer;
        lidar_scan[lidar_write_buffer].count = 0U;
    }
    else
    {
        /* 主循环处理不及时，丢弃本圈数据并继续接收，避免破坏正在处理的数据。 */
        lidar_scan[lidar_write_buffer].count = 0U;
    }
}

/**
  * @brief  检测扫描换圈并保存位于目标距离带内的雷达点。
  * @param  angle_cdeg 点角度，单位为 0.01°，范围为 0～35999。
  * @param  distance_mm 点距离，单位为 mm。
  * @note   所有点都参与换圈判断，但只有 500～600 mm 的点会写入扫描缓冲区。
  */
static void Lidar_AddPoint(uint16_t angle_cdeg, uint16_t distance_mm)
{
    Lidar_Scan *scan;

    if (lidar_have_last_angle &&
        (lidar_last_angle_cdeg > 30000U) &&
        (angle_cdeg < 6000U))
    {
        Lidar_CommitScan();
    }

    lidar_last_angle_cdeg = angle_cdeg;
    lidar_have_last_angle = 1U;

    if ((distance_mm < LIDAR_CAR_DISTANCE_MIN_MM) ||
        (distance_mm > LIDAR_CAR_DISTANCE_MAX_MM))
    {
        return;
    }

    scan = &lidar_scan[lidar_write_buffer];
    if (scan->count < LIDAR_MAX_SCAN_POINTS)
    {
        scan->points[scan->count].angle_cdeg = angle_cdeg;
        scan->points[scan->count].distance_mm = distance_mm;
        scan->count++;
    }
}

/**
  * @brief  校验并解析一个完整的 N10 数据帧。
  * @param  frame 固定 58 字节的数据帧。
  * @note   校验通过后，根据起止角度线性插值得到帧内 16 个测距点的角度。
  */
static void Lidar_DecodeFrame(const uint8_t frame[LIDAR_FRAME_LENGTH])
{
    uint8_t checksum = 0U;
    uint8_t i;
    uint16_t start_angle;
    uint16_t stop_angle;
    uint32_t angle_span;

    for (i = 0U; i < (LIDAR_FRAME_LENGTH - 1U); i++)
    {
        checksum = (uint8_t)(checksum + frame[i]);
    }

    if (checksum != frame[LIDAR_FRAME_LENGTH - 1U])
    {
        lidar_error_frame_count++;
        return;
    }

    start_angle = (uint16_t)(Lidar_ReadBE16(&frame[5]) % 36000U);
    stop_angle = (uint16_t)(Lidar_ReadBE16(&frame[55]) % 36000U);
    angle_span = (stop_angle >= start_angle)
               ? (uint32_t)(stop_angle - start_angle)
               : (uint32_t)(36000U - start_angle + stop_angle);

    for (i = 0U; i < LIDAR_POINTS_PER_FRAME; i++)
    {
        uint16_t offset = (uint16_t)(7U + (uint16_t)i * 3U);
        uint16_t distance_mm = Lidar_ReadBE16(&frame[offset]);
        uint32_t interpolated = (uint32_t)start_angle +
                                (angle_span * i + 7U) / 15U;
        uint16_t angle_cdeg = (uint16_t)(interpolated % 36000U);

        Lidar_AddPoint(angle_cdeg, distance_mm);
    }

    lidar_valid_frame_count++;
}

/**
  * @brief  判断两个相邻目标点是否属于同一个数据簇。
  * @param  left           左侧雷达点。
  * @param  right          右侧雷达点。
  * @param  angle_gap_cdeg 两点角度间隔，单位为 0.01°。
  * @retval 1：属于同一数据簇；0：两点之间为数据簇断点。
  * @note   小角度间隔直接连接；间隔较大时，仅在两侧距离相近的情况下跨越杆子遮挡。
  */
static uint8_t Lidar_PointsConnected(const Lidar_Point *left,
                                     const Lidar_Point *right,
                                     uint16_t angle_gap_cdeg)
{
    if (angle_gap_cdeg <= LIDAR_NORMAL_POINT_GAP_CDEG)
    {
        return 1U;
    }

    /* 杆子会挖掉中间若干点；角度间隙有限且两侧距离接近时仍视为同一簇。 */
    if ((angle_gap_cdeg <= LIDAR_OCCLUSION_GAP_CDEG) &&
        (Lidar_AbsDiffU16(left->distance_mm, right->distance_mm) <=
         LIDAR_OCCLUSION_DISTANCE_GAP_MM))
    {
        return 1U;
    }

    return 0U;
}

/**
  * @brief  按指定起点循环访问扫描点，并可将跨越 0° 的角度展开为连续角度。
  * @param  scan            待读取的整圈扫描数据。
  * @param  start_index     循环序列的起始下标。
  * @param  position        相对起始下标的位置。
  * @param  unwrapped_angle 展开角度输出地址；不需要角度时传入空指针。
  * @retval 指向对应雷达点的只读指针。
  */
static const Lidar_Point *Lidar_GetOrderedPoint(const Lidar_Scan *scan,
                                                uint16_t start_index,
                                                uint16_t position,
                                                uint32_t *unwrapped_angle)
{
    uint32_t raw_index = (uint32_t)start_index + position;
    uint16_t index = (uint16_t)(raw_index % scan->count);

    if (unwrapped_angle != 0)
    {
        *unwrapped_angle = scan->points[index].angle_cdeg;
        if (raw_index >= scan->count)
        {
            *unwrapped_angle += 36000U;
        }
    }

    return &scan->points[index];
}

/**
  * @brief  计算循环有序序列中当前点与下一点的角度间隔。
  * @param  scan        待读取的整圈扫描数据。
  * @param  start_index 循环序列的起始下标。
  * @param  position    当前点在循环序列中的位置。
  * @retval 相邻两点的角度间隔，单位为 0.01°。
  */
static uint16_t Lidar_OrderedGap(const Lidar_Scan *scan,
                                 uint16_t start_index,
                                 uint16_t position)
{
    uint32_t left_angle;
    uint32_t right_angle;

    (void)Lidar_GetOrderedPoint(scan, start_index, position, &left_angle);
    (void)Lidar_GetOrderedPoint(scan, start_index,
                                (uint16_t)(position + 1U), &right_angle);
    return (uint16_t)(right_angle - left_angle);
}

/**
  * @brief  对整圈目标点进行分簇并计算小车的角度位置。
  * @param  scan 已完成接收且不会再被中断修改的整圈扫描数据。
  * @note   选择有效点数最多且不少于 3 点的数据簇，取簇内角度中值作为
  *         car_angle，并发布平均距离、点数和有效标志。
  */
static void Lidar_FindCar(const Lidar_Scan *scan)
{
    uint16_t break_after = 0U;
    uint16_t ordered_start = 0U;
    uint16_t cluster_start = 0U;
    uint16_t cluster_count = 0U;
    uint16_t best_start = 0U;
    uint16_t best_count = 0U;
    uint32_t cluster_distance_sum = 0U;
    uint32_t best_distance_sum = 0U;
    uint16_t i;
    uint8_t found_break = 0U;

    if (scan->count < LIDAR_MIN_CLUSTER_POINTS)
    {
        car_angle_valid = 0U;
        car_distance_mm = 0U;
        lidar_target_point_count = 0U;
        return;
    }

    /* 从真实断点之后开始遍历，使跨越 0° 的同一目标仍归入一个数据簇。 */
    for (i = 0U; i < scan->count; i++)
    {
        uint16_t next = (uint16_t)((i + 1U) % scan->count);
        uint16_t gap = (next == 0U)
                     ? (uint16_t)(scan->points[0].angle_cdeg + 36000U -
                                  scan->points[i].angle_cdeg)
                     : (uint16_t)(scan->points[next].angle_cdeg -
                                  scan->points[i].angle_cdeg);

        if (!Lidar_PointsConnected(&scan->points[i], &scan->points[next], gap))
        {
            break_after = i;
            found_break = 1U;
            break;
        }
    }

    if (found_break)
    {
        ordered_start = (uint16_t)((break_after + 1U) % scan->count);
    }

    for (i = 0U; i < scan->count; i++)
    {
        const Lidar_Point *point = Lidar_GetOrderedPoint(scan, ordered_start, i, 0);
        uint8_t cluster_ends = (i == (uint16_t)(scan->count - 1U));

        cluster_count++;
        cluster_distance_sum += point->distance_mm;

        if (!cluster_ends)
        {
            const Lidar_Point *next = Lidar_GetOrderedPoint(scan, ordered_start,
                                                            (uint16_t)(i + 1U), 0);
            uint16_t gap = Lidar_OrderedGap(scan, ordered_start, i);
            cluster_ends = !Lidar_PointsConnected(point, next, gap);
        }

        if (cluster_ends)
        {
            if (cluster_count > best_count)
            {
                best_start = cluster_start;
                best_count = cluster_count;
                best_distance_sum = cluster_distance_sum;
            }

            cluster_start = (uint16_t)(i + 1U);
            cluster_count = 0U;
            cluster_distance_sum = 0U;
        }
    }

    /* 选择有效点最多的数据簇，并对簇内有序角度取中值。 */
    if (best_count >= LIDAR_MIN_CLUSTER_POINTS)
    {
        uint16_t middle = (uint16_t)((best_count - 1U) / 2U);
        uint32_t median_angle_1;
        uint32_t median_angle;

        (void)Lidar_GetOrderedPoint(scan, ordered_start,
                                    (uint16_t)(best_start + middle),
                                    &median_angle_1);
        median_angle = median_angle_1;

        if ((best_count & 1U) == 0U)
        {
            uint32_t median_angle_2;
            (void)Lidar_GetOrderedPoint(scan, ordered_start,
                                        (uint16_t)(best_start + middle + 1U),
                                        &median_angle_2);
            median_angle = (median_angle_1 + median_angle_2) / 2U;
        }

        median_angle %= 36000U;
        car_angle = (float)median_angle / 100.0f;
        car_distance_mm = (uint16_t)(best_distance_sum / best_count);
        lidar_target_point_count = best_count;
        car_angle_valid = 1U;
    }
    else
    {
        car_angle_valid = 0U;
        car_distance_mm = 0U;
        lidar_target_point_count = 0U;
    }
}

/**
  * @brief  初始化雷达帧解析、整圈双缓冲和识别结果。
  * @note   初始化完成后启动 USART1 单字节中断接收。
  */
void Lidar_Init(void)
{
    lidar_frame_index = 0U;
    lidar_scan[0].count = 0U;
    lidar_scan[1].count = 0U;
    lidar_write_buffer = 0U;
    lidar_ready_buffer = LIDAR_NO_BUFFER;
    lidar_processing_buffer = LIDAR_NO_BUFFER;
    lidar_have_last_angle = 0U;
    car_angle_valid = 0U;
    car_distance_mm = 0U;
    lidar_target_point_count = 0U;
    lidar_valid_frame_count = 0U;
    lidar_error_frame_count = 0U;

    (void)HAL_UART_Receive_IT(&huart1, &lidar_rx_byte, 1U);
}

/**
  * @brief  处理 USART1 收到的一个字节并维护 N10 帧接收状态机。
  * @note   本函数由 HAL_UART_RxCpltCallback() 在串口中断上下文中调用，
  *         每次处理结束后重新启动下一字节接收。
  */
void Lidar_UART_RxCpltCallback(void)
{
    uint8_t data = lidar_rx_byte;

    switch (lidar_frame_index)
    {
        case 0U:
            if (data == LIDAR_FRAME_HEADER_0)
            {
                lidar_frame[0] = data;
                lidar_frame_index = 1U;
            }
            break;

        case 1U:
            if (data == LIDAR_FRAME_HEADER_1)
            {
                lidar_frame[1] = data;
                lidar_frame_index = 2U;
            }
            else if (data != LIDAR_FRAME_HEADER_0)
            {
                lidar_frame_index = 0U;
            }
            break;

        case 2U:
            if (data == LIDAR_FRAME_LENGTH)
            {
                lidar_frame[2] = data;
                lidar_frame_index = 3U;
            }
            else
            {
                lidar_frame_index = 0U;
            }
            break;

        default:
            lidar_frame[lidar_frame_index++] = data;
            if (lidar_frame_index >= LIDAR_FRAME_LENGTH)
            {
                Lidar_DecodeFrame(lidar_frame);
                lidar_frame_index = 0U;
            }
            break;
    }

    (void)HAL_UART_Receive_IT(&huart1, &lidar_rx_byte, 1U);
}

/**
  * @brief  复位雷达帧状态并恢复 USART1 中断接收。
  * @note   本函数由统一的 HAL 串口错误回调调用，同时累计错误次数。
  */
void Lidar_UART_ErrorCallback(void)
{
    lidar_frame_index = 0U;
    lidar_error_frame_count++;
    __HAL_UART_CLEAR_OREFLAG(&huart1);
    (void)HAL_UART_Receive_IT(&huart1, &lidar_rx_byte, 1U);
}

/**
  * @brief  在主循环中提取一整圈待处理数据并更新小车识别结果。
  * @note   仅在交换双缓冲所有权时短暂关闭中断，分簇计算期间保持中断接收。
  */
void Lidar_Process(void)
{
    uint8_t buffer;

    __disable_irq();
    buffer = lidar_ready_buffer;
    if (buffer != LIDAR_NO_BUFFER)
    {
        lidar_ready_buffer = LIDAR_NO_BUFFER;
        lidar_processing_buffer = buffer;
    }
    __enable_irq();

    if (buffer == LIDAR_NO_BUFFER)
    {
        return;
    }

    Lidar_FindCar(&lidar_scan[buffer]);

    __disable_irq();
    lidar_processing_buffer = LIDAR_NO_BUFFER;
    __enable_irq();
}
