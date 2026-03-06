# TAMRA

**TAMRA** (**T**oolkit for **A**daptive **M**esh **R**efinement **A**pplications) is a modular **C++ HPC framework** designed to accelerate the development of **scalable adaptive numerical simulation codes**.

It provides the building blocks needed to add **adaptive mesh refinement (AMR)**, **distributed-memory parallelism**, and a flexible solver architecture to scientific applications, while keeping the numerical formulation of each solver as independent as possible.

TAMRA is designed for teams who want to:

- improve the scalability of existing simulation codes,
- reduce computational cost by refining only where needed,
- support both **Eulerian** and **Lagrangian** methods,
- build reusable multiphysics software instead of solver-specific prototypes.

---

## Why TAMRA

Large-scale simulations often face two major limitations:

1. **Computational cost grows too quickly** when the full domain is resolved uniformly.
2. **Codebases become difficult to maintain** when refinement, parallelization, and solver logic are too tightly coupled.

TAMRA addresses these issues by providing a framework where:

- **AMR reduces unnecessary computation** by concentrating resolution only in the important parts of the domain,
- **MPI-based domain decomposition improves scalability** on distributed-memory systems,
- solver-independent abstractions make it easier to support **2D/3D**, multiple discretizations, and multiphysics extensions.

The goal is not only to run simulations faster, but also to make advanced HPC simulation software easier to design, extend, and maintain.

---

## Main benefits

- **Computing acceleration through AMR**  
  Refine only where the physics requires it instead of using a uniformly fine mesh everywhere.

- **Improved scalability**  
  Built for distributed-memory architectures with MPI-based parallel execution.

- **Reusable software architecture**  
  Decouples mesh adaptation, parallel partitioning, and solver logic to simplify development of new solvers.

- **Support for multiple numerical paradigms**  
  Suitable for both **Eulerian grid-based methods** and **Lagrangian particle methods**.

- **Designed for research and industrial code evolution**  
  TAMRA can serve both as a basis for new simulation software and as a way to accelerate/refactor existing in-house codes.

---

## Target applications

TAMRA is intended for advanced scientific and engineering simulations, including for example:

- compressible and incompressible flow solvers,
- particle-based methods such as **SPH**,
- wave propagation solvers,
- multiphase and multiphysics simulations,
- research codes that need better scalability and adaptive resolution.

Examples of numerical approaches that can be integrated include:

- **Eulerian methods**: Roe-type finite-volume solvers, LBM, DG, finite differences,
- **Lagrangian methods**: SPH and related particle methods.

---

## Core ideas

TAMRA is being developed around a few core principles:

- **performance-oriented**
- **dimension-independent (2D / 3D)**
- **solver-agnostic**
- **modular and extensible**
- **adapted to large-scale distributed simulations**

---

## Current status

TAMRA is an active research and development project.

At this stage, the repository is primarily intended to present the framework and its direction.  
More material will be added progressively, including:

- improved user documentation,
- architecture explanations,
- tutorials and examples,
- benchmark cases,
- guidance for integrating TAMRA into existing solvers.

**Documentation is still under active development and will be expanded soon.**

---

## Getting started

### Prerequisites

- C++17 compatible compiler
- Eigen 3.4.0
- MPI environment for distributed runs

### Usage

TAMRA is intended to be integrated into scientific C++ projects requiring adaptive mesh refinement and scalable execution.

More complete setup instructions and examples will be provided in upcoming documentation updates.

---

## Documentation

Detailed documentation is **coming soon**.

Planned documentation includes:

- framework overview,
- API documentation,
- examples,
- integration notes for external solvers,
- performance/scalability discussions.

---

## Contributing

Contributions are welcome.

If you are interested in:

- contributing code,
- improving documentation,
- testing the framework,
- discussing solver integration,
- adding benchmark cases,

please open an issue or submit a pull request.

**Contributors are very welcome**, especially from the scientific computing, CFD, AMR, and HPC communities.

---

## Collaboration and industrial/research interest

TAMRA may also be of interest to:

- **research labs** developing in-house simulation codes,
- **companies** looking to improve the scalability of existing numerical software,
- teams interested in adding **AMR**, **MPI scalability**, or a cleaner solver architecture to their current codebase.

If your lab or company is interested in using TAMRA, collaborating, or exploring how it could help accelerate or modernize an existing simulation code, feel free to get in touch.

---

## License

This project is licensed under the MIT License.
