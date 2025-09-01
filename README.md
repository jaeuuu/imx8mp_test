# IMX8M-PLUS Test Program
- Copyright (c) 2025 jaewoo oh <<jwoh9511@gmail.com>>
- This T/P(Test Program) has written by ONPOOM Corp.

## Jig Board Test Program

- This T/P is implemented using ncurses library.
- Keyboard event applied.
- Mouse event will be applied.
- This T/P is applied console theme.

### 1. GPIO Test
- GPIO input control implemented.
- GPIO output control implemented.
- Refer to "gpioctl.c/.h"

### 2. UART Test
- Internal UART control implemented.
- PCIe-to-UART control implemented.
- Refer to "uartctl.c/.h"

### 3. NETWORK Test
- Ethernet1(eth0) Interface test implemented.
- Ethernet2(eth1) Interface test implemented.
- Refer to "netctl.c/.h"

### 4. CAN Test
- Controller Area Network(can0, can1) Interface test implemented.
- Refer to "canctl.c/.h"

### 5. LCD Test
- LVDS, MIPI-DSI for display interface test implemented.
- There is output and touch test.
- Refer to "lcdctl.c/.h"

### 6. CAMERA Test
- MIPI-CSI for camera interface test implemented.
- There is video streaming and capture test.
- Refer to "cameractl.c/.h"

### 7. AUDIO Test
- SAI2, SAI3(I2S Interface) for audio interface test implemented.
- There is audio streaming, tone playback and tone waveform test.
- Refer to "audioctl2.c/.h"

### 8. STRESS Test
- Stress of IMX8M-PLUS System on Module(SoM) test implemented.
- Used stress-ng command.
- A53 Core & Memory Load, Heating test implemented.
- Refer to "stressctl.c/.h"

### 9. USB Test
- Universal Serial Bus(USB) Intreface test implemented.
- There is USB flashing, USB information test.
- Refer to "usbctl.c/.h"

### 10. GPU Test
- Graphic Processing Unit(GPU) test implemented.
- There is 2D & 3D rendering test.
- Used glmark2-es2-wayland command.
- Refer to "gpuctl.c/.h"

