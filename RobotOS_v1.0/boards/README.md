# boards/

Thư mục chứa **Zephyr board overlay files** — tùy chỉnh DeviceTree cho từng phần cứng mục tiêu.

> Hiện tại thư mục này còn trống. Các overlay sẽ được thêm khi bắt đầu port lên board cụ thể.

---

## Cấu trúc dự kiến

```
boards/
├── nucleo_f411re.overlay   ← STM32F4 Nucleo board
├── esp32.overlay           ← ESP32 (Xtensa)
├── nrf52840dk.overlay      ← Nordic nRF52 DK
└── qemu_cortex_m3.overlay  ← QEMU virtual board (CI/CD)
```

---

## Nội dung một overlay file

```dts
/* boards/nucleo_f411re.overlay */

/ {
    steppers {
        compatible = "robotos,stepper-gpio";

        x_stepper: x_stepper {
            step-gpios  = <&gpioa 5 GPIO_ACTIVE_HIGH>;
            dir-gpios   = <&gpioa 6 GPIO_ACTIVE_HIGH>;
            enable-gpios= <&gpiob 1 GPIO_ACTIVE_LOW>;
        };

        y_stepper: y_stepper {
            step-gpios  = <&gpiob 3 GPIO_ACTIVE_HIGH>;
            dir-gpios   = <&gpiob 4 GPIO_ACTIVE_HIGH>;
            enable-gpios= <&gpiob 5 GPIO_ACTIVE_LOW>;
        };
    };

    endstops {
        x_min: x_min {
            gpios = <&gpioc 7 (GPIO_ACTIVE_LOW | GPIO_PULL_UP)>;
        };
    };
};

&usart2 {
    current-speed = <115200>;
    status = "okay";
};
```

---

## Build với overlay

```bash
west build -b nucleo_f411re . -- -DDTC_OVERLAY_FILE=boards/nucleo_f411re.overlay
```

Hoặc đặt tên đúng theo board (`<board>.overlay`) thì Zephyr tự load:

```bash
west build -b nucleo_f411re .
# Zephyr tự tìm boards/nucleo_f411re.overlay
```

---

## Board được hỗ trợ (roadmap)

| Board | MCU | Status |
|-------|-----|--------|
| `nucleo_f411re` | STM32F411 @ 100 MHz | Planned v0.1 |
| `esp32` | Xtensa LX6 @ 240 MHz | Planned v0.2 |
| `nrf52840dk` | nRF52840 @ 64 MHz | Planned v0.2 |
| `qemu_cortex_m3` | Virtual | CI/CD |
