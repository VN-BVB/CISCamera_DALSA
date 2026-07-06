# measurementMonitor 设计记录

> 如代码与本文档冲突，以代码为准并更新本文档。

## MVP 架构

```
View ──sig──▶ Presenter ──call──▶ Model (Pipeline)
  ▲                                       │
  └────────── display commands ───────────┘
```

| 层 | 类 | 职责 |
|---|---|---|
| View | `MeasurementMonitor` | 子 widget 信号 → forwarder signal；显示命令 |
| Presenter | `MeasurementPresenter` | 编排 View↔Model；维护 `m_lastImage` 与按钮状态机 |
| Model | `MeasurementPipeline` | 封装 `ImageReadWorker` + `ImageProcessWorker` + 2× QThread |

子 widget：`DataSourcePanel`（模式/文件/连接/执行）、`ResultDisplayPanel`（overlay flags/状态）、`LogPanel`、`FrmVisionDisplay`（图像 + 图形叠加）。

跨线程统一用 `QMetaObject::invokeMethod`；自定义类型在 Pipeline ctor 第一行 `qRegisterMetaType`。

---

## 路径 A：本地文件（已解耦）

选文件→显示原图；点按钮→处理。

```
[选文件]
  DataSourcePanel::startImageRead(path)
    → View::startImageReadRequested
      → Presenter::onStartImageReadRequested
        → setMeasurementEnabled(false) + readFromFile(path)
          → [read thread] ImageReadWorker::whenReadImage → cv::imread
            → sendImageRead(image)
              → Pipeline::onWorkerImageRead → emit imageRead
                → Presenter::onImageRead
                  → m_lastImage = image
                  → view->displayOriginalImage(image)   ← 仅显示
                  → setMeasurementEnabled(true)

[点执行测量]
  DataSourcePanel::executeSingleMeasurementRequested
    → View::executeSingleMeasurementRequested
      → Presenter::onExecuteSingleMeasurementRequested
        → setMeasurementEnabled(false) + processImage(m_lastImage)
          → [process thread] ImageProcessWorker::whenProcessImage → JointSeam::run()
            → imageProcessed(image, seam)
              → Pipeline::onWorkerImageProcessed → emit measurementCompleted
                → Presenter::onMeasurementCompleted
                  → view->displayMeasurementResult(image, seam)   ← 叠加 contour/line/point
                  → setMeasurementEnabled(true)
```

按钮状态机：初始禁用 → 读图中禁用 → 就绪启用 → 处理中禁用 → 完成/错误恢复（错误时按 `m_lastImage` 是否非空决定）。

---

## 路径 B：共享内存（未解耦）

一次连接完成读 + 处理；多 ROI 由 worker→worker 直通。

```
[点连接]
  DataSourcePanel::connectSharedMemoryRequested(pid)
    → View::startImageReadFromSharedMemoryRequested
      → Presenter::onStartImageReadFromSharedMemoryRequested
        → readFromSharedMemory(pid, 30000)
          → [read thread] ImageReadWorker::whenReadImageFromSharedMemory
              → QSharedMemory attach + 多 ROI 解析
              → sendImagesRead(rois)
                  ├── Pipeline::onWorkerImagesRead      (仅日志)
                  └── ImageProcessWorker::whenProcessMultiImages   ★ 直通
                       → JointSeam::run() per ROI
                       → imageProcessed(image, seam)
                         → Pipeline::onWorkerImageProcessed → emit measurementCompleted
                           → Presenter::onMeasurementCompleted → view->displayMeasurementResult
```

**直通点**（`models/measurement_pipeline.cpp:47`）：

```cpp
connect(m_readWorker, &ImageReadWorker::sendImagesRead,
        m_processWorker, &ImageProcessWorker::whenProcessMultiImages);
```

> 若将来要求 B 也"先显示后处理"：拆掉上述直通，Presenter 收 `imagesRead(rois)` 后存 `m_lastRois`，按钮按模式分流到 `processImage` / `processMultiImages`。

---

## A vs B

| | 路径 A | 路径 B |
|---|---|---|
| 输入 | 单图文件 | 多 ROI |
| 读图后 | 显示原图 | 立即处理 |
| 处理触发 | 显式按钮 | 隐式（自动） |
| `m_lastImage` | 用（可重测） | 不用 |
| 经 Presenter 中转 | 是 | 否（worker 直通） |

---

## 关键文件

```
src/ui/measurementMonitor/
├── measurement_monitor.h/.cpp          View
├── presenters/measurement_presenter.*  Presenter
├── models/measurement_pipeline.*       Model facade
└── widgets/                            子 widget

src/jointDetection/                     worker（被 Model 复用）
├── image_read_worker.*
├── image_process_worker.*
└── joint_seam.*                        检测算法
```

## MVP 当前装配

`main.cpp` 临时装配（`MeasurementMonitor + Pipeline + Presenter`），正式装配待 MainWindow 整合阶段。
