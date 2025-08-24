
> **Status:** In development

> **Goal:** Mesh-networked esp32 nodes communicating via ESP-NOW to send real time data and audio

---

## LICENSE
Apache License 2.0

## Overview

This is an unfinished ESP-IDF intercom project. The goal is to eventually let multiple esp32 nodes form a mesh and exchange audio and data to eachother using ESP-NOW and efficient routing. So far, it provides reusable components for:

- **UART input** (`uart_handler`)  
- **High-resolution timing** (`utility/timer`)  
- **OLED display (SSD1306)** (`display/display`)  
- **Menu/navigation UI** (`menu_system/menu_system`)
- **ADC DMA input** (`audio/adc_dma_input`)

Each component is demonstrated in its own example

## TODO

- **DMA output audio to DAC (I2S)** (`audio/I2S_dma_output`)
- **ESP-NOW mesh network**
- **Sleep mode to save power consumption** 

## Hardware 
- **esp32** (WROOM-32U for longer range)
- **DAC** (max98357A)
- **OLED** (SSD1306)
