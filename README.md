# Cellular Automaton Color Simulation

This is a C++ program that uses the [SFML](https://www.sfml-dev.org/) to create 
a visual simulation.

Each cell takes the color of its neighboring cells. A cell can have up to 8 neighbors. 
You can set the initial state.

Download it [here](https://github.com/dpieve/cellular-automaton/releases) and try it now! 

## 🎥 Showcase
Below is a brief description of how the project works and what it looks like when running:

* A grid of cells is initialized, each with a random color.
* Walls are created to indicate those cells will not change color.
* Each time the simulation updates, every cell selects one of its neighboring color (up, down, left, right, and diagonals).

Notice how the grid becomes more uniform with time.
After many iterations, the grid will be filled with a single color.

<p align="center">
<img src="assets/showcase.gif" width="800">
</p>

## 🛠️ Build and Run Instructions

### Prerequisites

* CMake (3.28+)
* Git
* C++ compiler

Clone the repository:

```bash

git clone https://github.com/dpieve/cellular-automaton.git
cd cellular-automaton
```

Create a new directory for the build and build it:

```bash
mkdir build
cd build
cmake ..
cmake --build .
```

Run the program:

```bash
./cellular-automaton
```

## 🤝 Contributions

Contributions are welcome! If you'd like to improve the project, whether by adding new features, 
optimizing the code, or fixing bugs, feel free to do it.

## 🧾 License

This project is licensed under the MIT License - see the [LICENSE](https://github.com/dpieve/cellular-automaton?tab=MIT-1-ov-file) file for details.