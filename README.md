# Optimized C++ Physics Simulation

An optimized physics simulation developed in C++ with a focus on low-level systems programming, cross-platform performance, and multithreaded design. Originally provided as an unoptimized base, this project involved transforming the simulation into a high-performance system capable of running on both Windows and PlayStation 4. Developed as part of a university module over the course of several weeks.

## 🎥 Demo

🎥 [Watch the Demo on YouTube](https://www.youtube.com/watch?v=S0nVa922c0Q) *(right-click to open in new tab)*  
*(Replace with actual link if you have one)*

## ⚙️ Features

- **Custom Memory Pool**  
  Reduces heap fragmentation and speeds up memory allocation.

- **Multithreaded Job System**  
  Executes physics updates in parallel, distributing tasks across CPU cores.

- **Dynamic Octree Spatial Partitioning**  
  Efficient collision detection with dynamic insertion/removal support.

- **Cross-Platform Optimization**  
  Runs efficiently on both Windows and PlayStation 4, with conditional compilation and platform-specific tuning.

- **Performance Profiling & Iterative Optimization**  
  Bottlenecks identified and resolved using platform tools to maintain real-time performance.

## 🛠 Technologies Used

- **Language:** C++  
- **Concurrency:** std::thread, mutexes, and custom job scheduling  
- **Memory Management:** Custom pool allocator  
- **Platforms:** Windows, PlayStation 4 (via SDK)  
- **Tools:** Visual Studio, PS4 development kit  
- **Project Duration:** ~8 weeks

## 🧠 What I Learned

This project provided hands-on experience with low-level systems programming, memory optimization, multithreading, and platform-specific development. Porting to PS4 and tuning for real-time performance challenged me to think critically about resource constraints and execution models.

## 📈 Future Improvements

- Add SIMD-based physics calculations for further performance gains  
- Extend the job system with dependency tracking and prioritization  
- Visualize the octree and collisions for debugging and analysis
