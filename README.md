# README for Demand and Material Prediction Application

# This project is a C++ application that predicts product demand and calculates required raw materials based on sales data stored in an SQLite database.
# It provides a user interface (UI) built using the FLTK (Fast Light Toolkit) and incorporates additional libraries like Eigen for numerical computations.

# ---

# Features:
# - Database Connectivity: Connects to an SQLite database to fetch sales and material data.
# - Linear Regression: Uses Eigen for linear regression to predict product demand.
# - Material Requirement Calculation: Computes raw material needs based on predicted demand.
# - User Interface: Interactive UI for inputting product IDs and viewing results.
# - Save as CSV: Allows exporting the prediction and material requirements to a CSV file.

# ---

# Libraries and Dependencies:
# Core Libraries:
# - FLTK: For building the graphical user interface (GUI).
# - SQLite3: For database management.
# - Eigen: For performing numerical computations like linear regression.
# C++ Standard Libraries:
# - <iostream>, <vector>, <fstream>, <cmath>: For input/output operations, data manipulation, and mathematical calculations.

# ---

# Database Structure:
# The application expects the following database schema.

# Table: Sales
# | Column      | Type   | Description                |
# |-------------|--------|----------------------------|
# | product_id  | INTEGER| Product ID                |
# | date        | TEXT   | Sale date (as YYYYMMDD)   |
# | sales       | REAL   | Quantity sold             |

# Table: Raw_Material
# | Column  | Type    | Description          |
# |---------|---------|----------------------|
# | id      | INTEGER | Material ID          |
# | name    | TEXT    | Raw material name    |

# Table: Conversion_Rate
# | Column           | Type    | Description                                  |
# |------------------|---------|----------------------------------------------|
# | product_id       | INTEGER | Product ID                                  |
# | raw_material_id  | INTEGER | Corresponding raw material ID               |
# | quantity_needed  | REAL    | Quantity of material required per unit sale |

# ---

# Build Instructions:
# Prerequisites:
# 1. Install the FLTK library.
# 2. Install SQLite3 development libraries.
# 3. Install the Eigen library.
# 4. A modern C++ compiler (e.g., g++ or clang++) supporting C++11 or later.

# Compiling the Project:
# Run the following command to build the project:

# g++ -std=c++11 -o demand_prediction main.cpp -lfltk -lfltk_images -lfltk_forms -lsqlite3

# Running the Application:
# After building the application, execute it using:

# ./demand_prediction

# How to Use:
# 1. Start the application: Run the compiled executable.
# 2. Enter Product ID: Input the product ID for which you want to predict demand.
# 3. View Predictions: Predicted demand will be displayed in the 'Predicted Demand' box.
# 4. Required materials will be listed in the 'Required Materials' box.
# 5. Save Results (Optional): Click the 'Save .CSV' button to export the results to a CSV file.

# ---

# Example Output:
# Predicted Demand
# Predicted Demand: 30
# Required Materials
# Raw Material: Quantity
# Wheat flour : 6000
# Butter : 2400
# Sugar : 3000

# Future Improvements:
# Add support for advanced predictive algorithms.
# Improve error reporting and user feedback.
# Enhance UI design for better user experience.
