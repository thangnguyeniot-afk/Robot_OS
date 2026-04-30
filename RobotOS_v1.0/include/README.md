# include/

Public header API cho toàn bộ RobotOS. Chia thành 2 subdirectory:

| Thư mục | Layer | Mô tả |
|---------|-------|-------|
| [`robotos/`](robotos/README.md) | Adapter + Framework | `ro_*.h` (HAL abstraction) + framework types (stepper, servo, PID…) |
| [`app/`](app/README.md) | Application | Types & interfaces dành riêng cho tầng ứng dụng |

## Quy tắc include

```c
// Adapter API — dùng trong Framework & App
#include <robotos/ro_thread.h>
#include <robotos/ro_gpio.h>

// Framework API — dùng trong App
#include <robotos/stepper.h>
#include <robotos/pid.h>

// App-specific types
#include <app/motion_seg.h>
#include <app/gcode_parser.h>
```

## Nguyên tắc phân tầng

- `robotos/*.h` KHÔNG được `#include <app/...>`.
- `app/*.h` có thể `#include <robotos/...>` nhưng không `#include` Zephyr trực tiếp.
- Implementation files trong `src/` mới được `#include` header của layer bên dưới.
