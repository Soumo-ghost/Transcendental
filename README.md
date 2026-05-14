# 3D Lorentz Force & Magnetic Mirror Simulator

Advance, highly interactive 3D computational physics engine built in C++ and Raylib. This simulation visualizes the behavior of charged particles in uniform electric fields and non-uniform magnetic topologies. It serves as a foundational tool for understanding electromagnetics, particle drift ($\vec{E} \times \vec{B}$), and plasma confinement.

---

## ✨ Key Features

- **Symplectic Physics Engine:** Utilizes the industry-standard **Boris Algorithm** for perfect energy conservation over infinite time frames, outperforming standard RK4 methods for magnetic fields.
- **Dynamic Magnetic Topologies:** Simulates a "Magnetic Bottle" with configurable base strength and spatial gradients ($\nabla \cdot \vec{B} = 0$ compliant).
- **Data Acquisition Pipeline:** Real-time telemetry streaming dumps 3D positional and velocity data to a `.csv` file for external data analysis in Python or MATLAB.
- **Custom Interactive UI:** A decoupled 2D control panel with smooth click-and-hold adjustments for live simulation tweaking without pausing.
- **Spherical Camera Controller:** Custom-built 3D camera that ensures stable viewing and precise click-and-drag rotation, independent of the UI.
- **Velocity-Mapped Trails:** Particle trails dynamically shift color (Blue $\rightarrow$ Red) based on real-time kinetic energy.
- **Energy Tracking:** A live UI readout calculating the exact Kinetic Energy (**$\frac{1}{2}mv^2$**) at every frame to prove that energy is being conserved properly.
- **Configuration Files:** Instead of hard-coding the starting positions, the program read a `.json` file on startup. This allows to set up 100 different starting scenarios without recompiling the C++ code  

---

![Alt Text] Simulation.gif

## 🔬 Theoretical Background

### The Lorentz Force & E $\times$ B Drift

The core of the simulation is governed by the Lorentz force law:

$$
\vec{F} = q(\vec{E} + \vec{v} \times \vec{B})
$$

When both an Electric Field ($\vec{E}$) and Magnetic Field ($\vec{B}$) are applied perpendicularly, the competing forces cause the particle's cycloid path to drift perpendicularly to *both* fields—a critical concept in Hall Effect sensors and mass spectrometers.

### Numerical Integration: Why the Boris Algorithm?

While first simulation was with the Runge-Kutta 4th Order (RK4) method , which is highly accurate for general differential equations, it is **not symplectic (A manifold is **not symplectic** if it fails to admit a closed, non-degenerate 2-form \(\omega \) due to Dimension)**. Over millions of integration steps, floating-point errors accumulate, causing simulated particles to artificially gain or lose energy.

This engine uses the **Boris Algorithm**—the absolute standard in plasma physicsand PIC codes because it offers excellent long-term energy conservation and accuracy. this method that cleanly separates the electric acceleration from the magnetic rotation. It perfectly conserves phase-space volume and the particle's kinetic energy over infinite time.

### The Magnetic Bottle (Mirror Effect)

To trap a particle in 3D space (as done in fusion reactors), we implement a **Magnetic Mirror**. We artificially increase the magnetic field strength at the edges of the simulation along the Z-axis. When the spiraling particle hits the high-density magnetic wall, the physics force it to compress, stop, and violently bounce back, effectively trapping the superheated plasma.

---

## 🎛️ Simulation Controls

### UI Panel

* **Pause / Resume:** Freeze the physics engine while keeping the camera active.
* **Reset Scene:** Return the particle to its initial state.
* **CSV Recording:** Toggle real-time data dumping to `telemetry_data.csv`.
* **Click-and-Hold Parameters:** Adjust Charge, Mass, E-Field (X/Y/Z), B-Field Base Strength, and Bottle Gradient dynamically.

### 3D Viewport

* **Rotate Camera:** `Left Click` + `Drag` anywhere in the 3D space.
* **Zoom:** `Mouse Wheel` scroll.

---

## 🚀 Building & Running

### Prerequisites

* A C++17 compatible compiler (e.g., GCC/MinGW on Windows).
* The [Raylib](https://www.raylib.com/) graphics library.

### Windows Setup (Using MSYS2 & VS Code)

1. Install Raylib from github and the GCC C++ compiler.
2. Compile and Simulate

### Linux Setup

1. Install Raylib dependencies: `sudo apt install libraylib-dev`
2. Compile:
   ```
   g++ -O2 main.cpp -o simulation -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
   ```

## 📊 Data Output

Clicking "START CSV RECORDING" in the UI generates a `telemetry_data.csv` file in the root directory formatted as follows:
`Time, PosX, PosY, PosZ, VelX, VelY, VelZ, KineticEnergy`

This file is ready for import into  MATLAB, or Excel for phase-space plotting and statistical analysis.


## 📝 License

This project is open-source and available under the MIT License. Feel free to fork, modify, and use it for your own educational or research purposes!

##### To commit to this project or any issue contact

Emiail - soumasishchakraborty02@gmail.com
