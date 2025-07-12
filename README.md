
> **Status:** In development
> **Goal:** Mesh-networked ESP32 nodes communicating via ESP-NOW with interactive screen allowing users to send and recieve data 

---

## Overview

This repository houses an ESP-IDF project that will eventually let multiple ESP32 nodes form a mesh and exchange data in real time. So far, it provides reusable components for:

- **UART input** (`uart_handler`)  
- **High-resolution timing** (`cycle_timer`)  
- **OLED display** (`display`)  
- **Menu/navigation UI** (`menu_system`)

Each component is demonstrated in its own example

---

## Prerequisites

- **Hardware:** ESP32-based board, ssd1306(optional)
- **Software:**  
  - ESP-IDF v4.x (tested on v4.4)  
  - Python 3.7+ for the ESP-IDF tools  
- **Libraries:**  
  - (built-in to ESP-IDF)

---