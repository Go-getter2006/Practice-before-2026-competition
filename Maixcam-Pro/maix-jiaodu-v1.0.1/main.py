from maix import camera, display, image, nn,uart,app,pinmap,touchscreen
detector = nn.YOLO11(model="/root/models/jiaodu_int8.mud", dual_buff = True) 
cam640 = camera.Camera(detector.input_width(), detector.input_height(), detector.input_format())
disp = display.Display()
ts = touchscreen.TouchScreen()
pinmap.set_pin_function("A17","UART0_RX")
pinmap.set_pin_function("A16","UART0_TX")
device="/dev/serial0"
serial=uart.UART(device,9600)
exit_btn = [400, 0, 240, 120]
sz=-1
mode=0
VOTE_LEN = 3          # 3帧投票
vote_buffer = []      # 保存最近结果
vote_threshold = 2    # 至少2票确认
last_vote_result = -1
def in_btn(x, y, r):
    return r[0] <= x <= r[0]+r[2] and r[1] <= y <= r[1]+r[3]
def vote_result(new_value):
    global vote_buffer

    # 加入最新结果
    vote_buffer.append(new_value)

    # 保持3帧窗口
    if len(vote_buffer) > VOTE_LEN:
        vote_buffer.pop(0)


    # 不足3帧
    if len(vote_buffer) < VOTE_LEN:
        return -1


    count = {}

    for v in vote_buffer:
        if v != -1:
            if v in count:
                count[v] += 1
            else:
                count[v] = 1


    if len(count)==0:
        return -1


    # =========================
    # 手动寻找最大票数
    # 替代 max(count,key=count.get)
    # =========================

    result = -1
    max_vote = 0

    for k in count:
        if count[k] > max_vote:
            max_vote = count[k]
            result = k


    if max_vote >= vote_threshold:
        return result

    return -1
while not app.need_exit():
    img = cam640.read()
    objs = detector.detect(img, conf_th = 0.5, iou_th = 0.45)
    if len(objs)>=2:
        continue
    for obj in objs:
        img.draw_rect(obj.x, obj.y, obj.w, obj.h, color = image.COLOR_RED)
        msg = f'{detector.labels[obj.class_id]}: {obj.score:.2f}'
        img.draw_string(obj.x, obj.y, msg, color = image.COLOR_RED)
        sss=int(detector.labels[obj.class_id])

        current_sz=-1

        if sss==90:
            current_sz=1
        elif sss==180:
            current_sz=2
        elif sss==270:
            current_sz=3
        elif sss==360:
            current_sz=4


# 加入投票
        vote_sz = vote_result(current_sz)


        if vote_sz!=-1:
            sz=vote_sz
            if sz!=-1 and mode==0:
                BCC=0xFF^0x01^sz
                serial.write(bytes([0xFF, 0x01,sz,BCC, 0xFE]))
                print([0xFF, 0x01,sz,BCC, 0xFE])
                mode=1
    img.draw_string(0,20,f"{sz}",image.Color.from_rgb(0,0,0),3)
    img.draw_rect(exit_btn[0], exit_btn[1], exit_btn[2], exit_btn[3],
                  image.Color.from_rgb(255, 0, 0), 2)
    img.draw_string(400, 20, "EXIT", image.Color.from_rgb(255, 0, 0),6)

    # 读取触摸
    x, y, pressed = ts.read()
    if pressed and in_btn(x, y, exit_btn):
        app.set_exit_flag(True)   # ⭐关键：退出程序
    disp.show(img)