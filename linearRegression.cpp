#include <iostream>
#include <sqlite3.h>
#include <vector>
#include <Eigen/Dense>
#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_File_Chooser.H>
#include <FL/Fl_Multiline_Output.H>
#include <fstream>
#include <cmath>

using namespace std;
using namespace Eigen;

// Function to connect to the database
sqlite3* connectDB(const char* dbName) {
    sqlite3* db;
    if (sqlite3_open(dbName, &db)) {
        cerr << "Error opening database: " << sqlite3_errmsg(db) << endl;
        return nullptr;
    }
    return db;
}

// Function to fetch sales data for a given product
vector<pair<int, double>> getSalesData(sqlite3* db, int product_id) {
    sqlite3_stmt* stmt;
    vector<pair<int, double>> salesData;
    const char* query = "SELECT date, sales FROM Sales WHERE product_id = ?;";
    if (sqlite3_prepare_v2(db, query, -1, &stmt, 0) != SQLITE_OK) {
        cerr << "Error preparing query: " << sqlite3_errmsg(db) << endl;
        return salesData;
    }
    sqlite3_bind_int(stmt, 1, product_id);
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const unsigned char* dateText = sqlite3_column_text(stmt, 0);
        if (dateText == nullptr) {
            cerr << "Null date found for product ID: " << product_id << endl;
            continue;
        }
        double sales = sqlite3_column_double(stmt, 1);
        int dateInt = stoi(reinterpret_cast<const char*>(dateText));
        salesData.push_back(make_pair(dateInt, sales));
    }
    sqlite3_finalize(stmt);
    return salesData;
}

// Function to perform linear regression
double linearRegression(vector<pair<int, double>>& data) {
    int n = data.size();
    if (n < 2) {
        return 0; // Regression cannot be performed with less than 2 data points
    }
    int firstDate = data[0].first;
    for (auto& entry : data) {
        entry.first = entry.first - firstDate; // Normalize dates for computation
    }

    MatrixXd X(n, 2);
    VectorXd Y(n);
    for (int i = 0; i < n; ++i) {
        X(i, 0) = 1; // Intercept term
        X(i, 1) = data[i].first; // Date difference
        Y(i) = data[i].second; // Sales value
    }

    VectorXd theta = (X.transpose() * X).ldlt().solve(X.transpose() * Y);

    return theta(0) + theta(1) * (data[n - 1].first); // Predict sales for the most recent date
}

// Function to calculate material requirements based on predicted demand
vector<pair<string, double>> calculateMaterialNeeds(sqlite3* db, int product_id, double predictedDemand) {
    sqlite3_stmt* stmt;
    const char* query = "SELECT Raw_Material.name, Conversion_Rate.quantity_needed "
                        "FROM Conversion_Rate "
                        "JOIN Raw_Material ON Conversion_Rate.raw_material_id = Raw_Material.id "
                        "WHERE Conversion_Rate.product_id = ?;";
    vector<pair<string, double>> materialNeeds;
    if (sqlite3_prepare_v2(db, query, -1, &stmt, 0) != SQLITE_OK) {
        cerr << "Error preparing query: " << sqlite3_errmsg(db) << endl;
        return materialNeeds;
    }
    sqlite3_bind_int(stmt, 1, product_id);
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        string materialName = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        double conversionRate = sqlite3_column_double(stmt, 1);
        double requiredQuantity = round(predictedDemand * conversionRate);
        materialNeeds.push_back(make_pair(materialName, requiredQuantity));
    }
    sqlite3_finalize(stmt);
    return materialNeeds;
}

// Function to save data to a CSV file
void saveToCSV(const string& demand, const vector<pair<string, double>>& materialNeeds) {
    const char* filePath = fl_file_chooser("Save As", "*.csv", "");
    if (filePath) {
        ofstream outFile(filePath);
        outFile << "Product,Predicted Demand\n";
        outFile << "Product," << demand << "\n";
        outFile << "\nRequired Materials\n";
        outFile << "Raw Material,Quantity\n";
        for (const auto& material : materialNeeds) {
            outFile << material.first << "," << material.second << "\n";
        }
        outFile.close();
    }
}

// Function to calculate prediction and display results in the UI
void calculatePrediction(Fl_Input* input, Fl_Multiline_Output* demandBox, Fl_Multiline_Output* materialBox, Fl_Button* saveButton) {
    sqlite3* db = connectDB("production.db");
    if (!db) return;

    int product_id = stoi(input->value());

    vector<pair<int, double>> salesData = getSalesData(db, product_id);
    if (salesData.empty()) {
        demandBox->value("No sales data");
        materialBox->value("No material data");
        sqlite3_close(db);
        saveButton->deactivate();
        return;
    }

    double predictedDemand = round(linearRegression(salesData));
    string demandStr = "Predicted Demand: " + to_string(static_cast<int>(predictedDemand));
    demandBox->value(demandStr.c_str());

    vector<pair<string, double>> materialNeeds = calculateMaterialNeeds(db, product_id, predictedDemand);
    string materialStr = "Required Materials:\n";
    for (const auto& material : materialNeeds) {
        materialStr += material.first + ": " + to_string(static_cast<int>(material.second)) + "\n";
    }
    materialBox->value(materialStr.c_str());

    saveButton->activate();
    sqlite3_close(db);
}

// Main function
int main() {
    Fl_Window* window = new Fl_Window(600, 550, "Demand and Material Prediction");
    Fl_Input* input = new Fl_Input(200, 60, 200, 30, "Product ID:");
    Fl_Multiline_Output* demandBox = new Fl_Multiline_Output(150, 120, 300, 30, "Predicted Demand:");
    Fl_Multiline_Output* materialBox = new Fl_Multiline_Output(150, 170, 300, 200, "Required Materials:");
    Fl_Button* button = new Fl_Button(200, 400, 200, 30, "Calculate");
    Fl_Button* saveButton = new Fl_Button(200, 450, 200, 30, "Save .CSV");

    saveButton->deactivate();

    button->callback([](Fl_Widget*, void* v) {
        Fl_Input* input = ((Fl_Input**)v)[0];
        Fl_Multiline_Output* demandBox = ((Fl_Multiline_Output**)v)[1];
        Fl_Multiline_Output* materialBox = ((Fl_Multiline_Output**)v)[2];
        Fl_Button* saveButton = ((Fl_Button**)v)[3];
        calculatePrediction(input, demandBox, materialBox, saveButton);
    }, new Fl_Widget* [4]{input, demandBox, materialBox, saveButton});

    saveButton->callback([](Fl_Widget*, void* v) {
        Fl_Multiline_Output* demandBox = ((Fl_Multiline_Output**)v)[0];
        Fl_Multiline_Output* materialBox = ((Fl_Multiline_Output**)v)[1];
        string demand = demandBox->value();
        
        vector<pair<string, double>> materialNeeds;
        stringstream ss(materialBox->value());
        string line;
        getline(ss, line);

        while (getline(ss, line)) {
            size_t pos = line.find(": ");
            if (pos != string::npos) {
                string material = line.substr(0, pos);
                double quantity = stoi(line.substr(pos + 2));
                materialNeeds.push_back(make_pair(material, quantity));
            }
        }

        saveToCSV(demand, materialNeeds);
    }, new Fl_Widget* [2]{demandBox, materialBox});

    window->end();
    window->show();
    return Fl::run();
}
