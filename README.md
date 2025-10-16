# Smart-Home-Device-Log-Conversion-System

This project is a command-line tool developed in C for converting smart home device logs between CSV, Binary, and XML formats. It also includes functionality for validating the generated XML files against a custom XSD schema. 

The tool demonstrates proficiency in file I/O operations in C, data serialization/deserialization, parsing different file formats, handling endianness, and integrating external libraries for JSON and XML processing.

## Features

- **CSV to Binary Conversion:** Reads log data from a CSV file (`smartlog.csv`), supports various delimiters (comma, tab, semicolon), and handles different line endings (Windows, Linux, macOS). It then serializes each record into a compact binary format and saves it to `logdata.dat`.

- **Binary to XML Conversion:** Reads the binary data from `logdata.dat` and, based on configuration parameters from a `setupParams.json` file, generates a structured XML file.
  - Supports sorting records based on a key field in either ascending (`ASC`) or descending (`DESC`) order.
  - Utilizes `libjson` for parsing the JSON configuration and `libxml2` for generating the XML output.

- **XML Validation with XSD:** Generates an XML Schema Definition (XSD) file (`smartlogs.xsd`) that defines the structure, constraints, patterns, and data types for the XML logs.
  - The tool can validate a given XML file against this XSD schema to ensure correctness.

- **Endianness Handling:** Correctly processes and represents multi-byte data (like `event_code`) in both Big Endian and Little Endian formats, a key requirement for data interchange between different system architectures.

- **Command-Line Interface:** Provides a flexible and user-friendly CLI with support for required arguments (`-separator`, `-opsys`) and a help flag (`-h`).

## Technologies and Libraries Used

- **Language:** C
- **Libraries:**
  - `libjson`: For parsing the `setupParams.json` configuration file.
  - `libxml2`: For creating the XML output file and performing XSD validation.
- **File Formats Handled:** CSV, Binary, XML, JSON
- **Schema:** XSD (XML Schema Definition)

## Project Structure

- `main.c`: The main source file containing the C implementation of the conversion and validation logic.
- `smartlogs.xsd`: The XML Schema Definition file used to validate the structure and content of the output XML file.
- `smartlog.csv` (Sample Input): A sample CSV file containing smart thermostat logs.
- `setupParams.json` (Sample Config): A sample JSON file to configure the binary-to-XML conversion.
- `validate.c` (Provided): A utility to integrate XSD validation with the main code.

## Compilation and Usage

### Prerequisites

You need to have `gcc` (or another C compiler), `libjson`, and `libxml2` development libraries installed on your system.

On a Debian/Ubuntu based system, you can install them using:
```bash
sudo apt-get update
sudo apt-get install gcc libjson-c-dev libxml2-dev

To compile the project, link the required libraries:
gcc main.c validate.c -o deviceTool -ljson-c -lxml2
Usage Examples
Convert CSV to Binary:
./deviceTool smartlog.csv logdata.dat 1 -separator 1 -opsys 2
Convert Binary to XML:
(Reads binary file name from setupParams.json)
./deviceTool smartlogs.xml 2 -separator 1 -opsys 2
Validate XML with XSD:
./deviceTool smartlogs.xml smartlogs.xsd 3 -separator 1 -opsys 2
Display Help Message:
./deviceTool -h
