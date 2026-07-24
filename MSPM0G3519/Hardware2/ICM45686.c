#include "ICM45686.h"
#include "ti_msp_dl_config.h"
#include "Delay.h"

/* ICM45686 本驱动用到的寄存器地址 */
#define ICM45686_REG_ACCEL_DATA_X1          (0x00U)
#define ICM45686_REG_PWR_MGMT0              (0x10U)
#define ICM45686_REG_INT1_STATUS0           (0x19U)
#define ICM45686_REG_ACCEL_CONFIG0          (0x1BU)
#define ICM45686_REG_GYRO_CONFIG0           (0x1CU)
#define ICM45686_REG_INTF_CONFIG1_OVRD      (0x2DU)
#define ICM45686_REG_DRIVE_CONFIG0          (0x32U)
#define ICM45686_REG_MISC2                  (0x7FU)

/* 量程、输出速率和工作模式配置值 */
#define ICM45686_PWR_MGMT0_LN_LN            (0x0FU)
#define ICM45686_ACCEL_CONFIG0_4G_200HZ     (0x38U)
#define ICM45686_GYRO_CONFIG0_1000DPS_200HZ (0x28U)

/* 原始数据单位换算系数 */
#define ICM45686_ACCEL_4G_RAW_TO_MG(raw)       (((float)(raw) * 4000.0f) / 32768.0f)
#define ICM45686_GYRO_1000DPS_RAW_TO_DPS(raw) (((float)(raw) * 1000.0f) / 32768.0f)

/*******************************************************************************
 * 名    称： ICM45686_ChipSelect
 * 功    能：控制 CS_IMU 片选引脚，低电平选中芯片，高电平释放总线。
 * 参    数：select：1 表示选中芯片，0 表示释放芯片。
 * 出    口：无返回值。
 *******************************************************************************/
static void ICM45686_ChipSelect(uint8_t select)
{
    if (select) {
        DL_GPIO_clearPins(IMU_PORT, IMU_CS_IMU_PIN);
    } else {
        DL_GPIO_setPins(IMU_PORT, IMU_CS_IMU_PIN);
    }
}

/*******************************************************************************
 * 名    称： ICM45686_TransferByte
 * 功    能：通过 SPI_IMU 发送 1 个字节，并同步读取 1 个返回字节。
 * 参    数：data：需要发送到 SPI 总线的字节。
 * 出    口：uint8_t，SPI 总线上同步接收到的字节。
 *******************************************************************************/
static uint8_t ICM45686_TransferByte(uint8_t data)
{
    uint8_t rx_data;

    DL_SPI_transmitData8(SPI_IMU_INST, data);
    while (DL_SPI_isBusy(SPI_IMU_INST)) { }

    rx_data = DL_SPI_receiveData8(SPI_IMU_INST);
    while (DL_SPI_isBusy(SPI_IMU_INST)) { }

    return rx_data;
}

/*******************************************************************************
 * 名    称： ICM45686_ReadRegs
 * 功    能：从指定寄存器地址开始连续读取多个字节。
 * 参    数：reg 为起始寄存器；buf 为接收缓冲区；len 为读取字节数。
 * 出    口：无返回值，读取结果通过 buf 输出。
 *******************************************************************************/
static void ICM45686_ReadRegs(uint8_t reg, uint8_t *buf, uint32_t len)
{
    ICM45686_ChipSelect(1U);
    ICM45686_TransferByte(reg | 0x80U);

    while (len--) {
        *buf++ = ICM45686_TransferByte(0x00U);
    }

    ICM45686_ChipSelect(0U);
}

/*******************************************************************************
 * 名    称： ICM45686_WriteReg
 * 功    能：向指定寄存器写入 1 个字节。
 * 参    数：reg 为寄存器地址；value 为写入值。
 * 出    口：无返回值。
 *******************************************************************************/
static void ICM45686_WriteReg(uint8_t reg, uint8_t value)
{
    ICM45686_ChipSelect(1U);
    ICM45686_TransferByte(reg & 0x7FU);
    ICM45686_TransferByte(value);
    ICM45686_ChipSelect(0U);
}

/*******************************************************************************
 * 名    称： ICM45686_CheckWhoAmI
 * 功    能：读取 WHO_AM_I 寄存器并与 ICM45686 的期望 ID 进行比较。
 * 参    数：无。
 * 出    口：uint8_t，1 表示芯片 ID 正确，0 表示读取失败或 ID 不匹配。
 *******************************************************************************/
uint8_t ICM45686_CheckWhoAmI(void)
{
    uint8_t whoami = 0U;

    ICM45686_ReadRegs(ICM45686_REG_WHO_AM_I, &whoami, 1U);

    return (whoami == ICM45686_WHOAMI_VALUE) ? 1U : 0U;
}

/*******************************************************************************
 * 名    称： ICM45686_SoftReset
 * 功    能：触发芯片软复位，并恢复复位前需要保留的接口相关寄存器。
 * 参    数：无。
 * 出    口：uint8_t，1 表示复位完成标志有效，0 表示复位失败。
 *******************************************************************************/
static uint8_t ICM45686_SoftReset(void)
{
    uint8_t intf_config1_ovrd;
    uint8_t drive_config0;
    uint8_t int1_status0;

    ICM45686_ReadRegs(ICM45686_REG_INTF_CONFIG1_OVRD, &intf_config1_ovrd, 1U);
    ICM45686_ReadRegs(ICM45686_REG_DRIVE_CONFIG0, &drive_config0, 1U);

    ICM45686_WriteReg(ICM45686_REG_MISC2, 0x02U);
    Delay_us(1000U);

    ICM45686_WriteReg(ICM45686_REG_DRIVE_CONFIG0, drive_config0);
    ICM45686_WriteReg(ICM45686_REG_INTF_CONFIG1_OVRD, intf_config1_ovrd);

    ICM45686_ReadRegs(ICM45686_REG_INT1_STATUS0, &int1_status0, 1U);

    return ((int1_status0 & 0x80U) != 0U) ? 1U : 0U;
}

/*******************************************************************************
 * 名    称： ICM45686_Init
 * 功    能：初始化 ICM45686，配置加速度计和陀螺仪为低噪声工作模式。
 * 参    数：无。
 * 出    口：uint8_t，1 表示初始化成功，0 表示初始化失败。
 *******************************************************************************/
uint8_t ICM45686_Init(void)
{
    if (!ICM45686_CheckWhoAmI()) {
        return 0U;
    }

    if (!ICM45686_SoftReset()) {
        return 0U;
    }

    ICM45686_WriteReg(ICM45686_REG_ACCEL_CONFIG0, ICM45686_ACCEL_CONFIG0_4G_200HZ);
    ICM45686_WriteReg(ICM45686_REG_GYRO_CONFIG0, ICM45686_GYRO_CONFIG0_1000DPS_200HZ);
    ICM45686_WriteReg(ICM45686_REG_PWR_MGMT0, ICM45686_PWR_MGMT0_LN_LN);

    Delay_ms(70U);

    return 1U;
}

/*******************************************************************************
 * 名    称： ICM45686_ReadAccel
 * 功    能：读取三轴加速度寄存器，并换算为 mg 单位。
 * 参    数：accel_mg：长度为 3 的输出数组，依次保存 X/Y/Z 三轴加速度。
 * 出    口：无返回值，读取结果通过 accel_mg 输出。
 *******************************************************************************/
void ICM45686_ReadAccel(float accel_mg[3])
{
    uint8_t raw[6];
    int16_t accel_raw[3];

    ICM45686_ReadRegs(ICM45686_REG_ACCEL_DATA_X1, raw, 6U);

    accel_raw[0] = (int16_t)((raw[1] << 8) | raw[0]);
    accel_raw[1] = (int16_t)((raw[3] << 8) | raw[2]);
    accel_raw[2] = (int16_t)((raw[5] << 8) | raw[4]);

    accel_mg[0] = ICM45686_ACCEL_4G_RAW_TO_MG(accel_raw[0]);
    accel_mg[1] = ICM45686_ACCEL_4G_RAW_TO_MG(accel_raw[1]);
    accel_mg[2] = ICM45686_ACCEL_4G_RAW_TO_MG(accel_raw[2]);
}

/*******************************************************************************
 * 名    称： ICM45686_ReadAccelGyro
 * 功    能：连续读取加速度计和陀螺仪数据，并分别换算为 mg 和 dps。
 * 参    数：accel_mg 保存三轴加速度；gyro_dps 保存三轴角速度。
 * 出    口：无返回值，读取结果通过两个数组输出。
 *******************************************************************************/
void ICM45686_ReadAccelGyro(float accel_mg[3], float gyro_dps[3])
{
    uint8_t raw[12];
    int16_t accel_raw[3];
    int16_t gyro_raw[3];

    ICM45686_ReadRegs(ICM45686_REG_ACCEL_DATA_X1, raw, 12U);

    accel_raw[0] = (int16_t)((raw[1]  << 8) | raw[0]);
    accel_raw[1] = (int16_t)((raw[3]  << 8) | raw[2]);
    accel_raw[2] = (int16_t)((raw[5]  << 8) | raw[4]);

    gyro_raw[0]  = (int16_t)((raw[7]  << 8) | raw[6]);
    gyro_raw[1]  = (int16_t)((raw[9]  << 8) | raw[8]);
    gyro_raw[2]  = (int16_t)((raw[11] << 8) | raw[10]);

    accel_mg[0] = ICM45686_ACCEL_4G_RAW_TO_MG(accel_raw[0]);
    accel_mg[1] = ICM45686_ACCEL_4G_RAW_TO_MG(accel_raw[1]);
    accel_mg[2] = ICM45686_ACCEL_4G_RAW_TO_MG(accel_raw[2]);

    gyro_dps[0] = ICM45686_GYRO_1000DPS_RAW_TO_DPS(gyro_raw[0]);
    gyro_dps[1] = ICM45686_GYRO_1000DPS_RAW_TO_DPS(gyro_raw[1]);
    gyro_dps[2] = ICM45686_GYRO_1000DPS_RAW_TO_DPS(gyro_raw[2]);
}
