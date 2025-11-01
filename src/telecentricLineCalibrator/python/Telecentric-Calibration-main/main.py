from Calibrator.calibrator import Calibrator
from corner_detector import PatternInfo
import time


if __name__=="__main__":
    # # # ----------HALCON------------------
    # img_dir = "D:/Code/CISCamera_DALSA/data/CISCamera_Image/txt"
    # pattern_type = PatternInfo(1, (7,7), 20, (25400/1200,25400/1200),10)
    # print("远心相机开始标定！")
    # s=time.time()
    # camera = Calibrator(img_dir,pattern_type,1.00026861,visualization=False)
    # camera.calibrate_camera()
    # e=time.time()
    # print(f"耗费时间：{e-s}s")
    # -----------CHESSBOARD-------------
    img_dir = "image/chessboard"
    pattern_type = PatternInfo(0, (8,11), 10, (25400/1200,25400/1200),0)
    print("远心相机开始标定！")
    s=time.time()
    camera = Calibrator(img_dir,pattern_type,1,visualization=False)
    camera.calibrate_camera()
    e=time.time()
    print(f"耗费时间：{e-s}s")

