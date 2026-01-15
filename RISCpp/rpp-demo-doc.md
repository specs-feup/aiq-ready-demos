# RISCpp MAC-NVISION Test Application Guide

This guide explains how to run the MAC-NVISION test application on the RISCpp soft-core processor.

## Repository Setup

To prepare the environment, you need to clone the RISCpp repository and switch to the specific extension branch.

1.  Clone the repository:
    ```bash
    git clone https://github.com/specs-feup/RISCpp.git
    cd RISCpp
    ```

2.  Switch to the `MAC-ext` branch:
    ```bash
    git checkout MAC-ext
    ```

## Documentation

The RISCpp repository provides comprehensive documentation covering the entire flow from application compilation to hardware synthesis and FPGA deployment. Please refer to the [Main Documentation](https://github.com/specs-feup/RISCpp/blob/main/README.md) for detailed instructions on:

*   **Getting Started**: Initial setup and requirements.
*   **Application Compilation**: How to compile C/C++ applications for the RISC-V core.
*   **Synthesis and Integration**: Steps to synthesize the soft-core and integrate it into your design.

## Running the Application

This guide specifically targets the **MAC-NVISION** application.

The source code for this application is located in:
`src/tests/MAC-NVISION`

To run this application, follow the standard execution/compilation procedures outlined in the main documentation of RISCpp, ensuring you point to the `src/tests/MAC-NVISION` directory as your application source.
