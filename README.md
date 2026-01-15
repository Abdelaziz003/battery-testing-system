# 🔋 Battery Multi-Channel Discharge Test System

A complete **embedded + desktop** solution for testing and analyzing battery discharge behavior using **Arduino (C/C++)** and **Python**.

---

## ✨ Overview

This project is designed to **test, log, and visualize battery discharge data** across multiple channels. It supports both **timed tests** and **full discharge tests**, with real-time measurements and post-test analysis via a desktop GUI.

---

## 🧠 System Architecture

* **Arduino (Embedded C/C++)**

  * Controls relays for load switching
  * Measures:

    * Battery voltage (loaded & unloaded)
    * Shunt voltages (high / low)
    * Discharge current
    * Accumulated capacity (mAh)
  * Sends structured data over **Serial**

* **Python Desktop Application**

  * Receives and parses serial data
  * Stores measurements in **JSON** format
  * Provides a **GUI dashboard** for:

    * Viewing raw data in tables
    * Plotting Voltage, Current, and Capacity vs Time

---

## ⚙️ Features

* 🔌 Multi-battery support (scalable design)
* ⏱️ Timed discharge test (default or custom duration)
* 🔻 Full discharge test (until voltage threshold, e.g. 3.0V)
* 📈 Real-time & post-test visualization
* 🧪 Pre-test and post-test measurements (no-load)
* 📁 Data persistence using JSON files

---

## 🖥️ Python GUI Modules

* `battery_viewer.py`
  Displays detailed data for a **single battery**, including:

  * Interactive table (expandable)
  * Graphs: Current, Capacity, Voltage vs Time

* `group_viewer.py`
  Overview and comparison of **multiple batteries**

* `battery_app.py`
  Main application logic and UI launcher

---

## 📡 Serial Command Interface

Examples:

```text
START:1,2        -> Timed test (default 30s)
START:1,2:60     -> Timed test (60 seconds)
FULLTEST:1,2     -> Full discharge until cutoff voltage
```

---

## 🛠️ Technologies Used

* **Embedded:** Arduino, C/C++
* **Desktop:** Python 3
* **GUI:** Tkinter, CustomTkinter
* **Visualization:** Matplotlib
* **Data Format:** JSON

---

## 🎯 Use Cases

* Battery capacity validation
* Load testing & discharge profiling
* Educational embedded systems projects
* R&D prototyping for power systems

---

## 👨‍💻 Author

Developed by an **Embedded Systems & C programmer** with experience in:

* Low-level hardware interfacing
* Serial communication protocols
* Data acquisition & visualization

---

✅ *This project demonstrates strong skills in Embedded Systems, C programming, hardware-software integration, and Python-based tooling.*
