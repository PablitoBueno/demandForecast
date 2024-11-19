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

// Função para conectar ao banco de dados
sqlite3* connectDB(const char* dbName) {
    sqlite3* db;
    if (sqlite3_open(dbName, &db)) {
        cerr << "Erro ao abrir o banco de dados: " << sqlite3_errmsg(db) << endl;
        return nullptr;
    }
    return db;
}

// Função para obter os dados de vendas
vector<pair<int, double>> getSalesData(sqlite3* db, int produto_id) {
    sqlite3_stmt* stmt;
    vector<pair<int, double>> salesData;
    const char* query = "SELECT data, vendas FROM Vendas WHERE produto_id = ?;";
    if (sqlite3_prepare_v2(db, query, -1, &stmt, 0) != SQLITE_OK) {
        cerr << "Erro ao preparar query: " << sqlite3_errmsg(db) << endl;
        return salesData;
    }
    sqlite3_bind_int(stmt, 1, produto_id);
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const unsigned char* dateText = sqlite3_column_text(stmt, 0);
        if (dateText == nullptr) {
            cerr << "Data nula encontrada para o produto ID: " << produto_id << endl;
            continue;
        }
        double sales = sqlite3_column_double(stmt, 1);
        int dateInt = stoi(reinterpret_cast<const char*>(dateText));
        salesData.push_back(make_pair(dateInt, sales));
    }
    sqlite3_finalize(stmt);
    return salesData;
}

// Função para realizar a regressão linear
double linearRegression(vector<pair<int, double>>& data) {
    int n = data.size();
    if (n < 2) {
        return 0;
    }
    int firstDate = data[0].first;
    for (auto& entry : data) {
        entry.first = entry.first - firstDate;
    }

    MatrixXd X(n, 2);
    VectorXd Y(n);
    for (int i = 0; i < n; ++i) {
        X(i, 0) = 1;
        X(i, 1) = data[i].first;
        Y(i) = data[i].second;
    }

    VectorXd theta = (X.transpose() * X).ldlt().solve(X.transpose() * Y);

    return theta(0) + theta(1) * (data[n - 1].first);
}

// Função para calcular as necessidades de matéria-prima
vector<pair<string, double>> calculateMaterialNeeds(sqlite3* db, int produto_id, double predictedDemand) {
    sqlite3_stmt* stmt;
    const char* query = "SELECT Materia_Prima.nome, Taxa_Conversao.quantidade_materia_prima "
                        "FROM Taxa_Conversao "
                        "JOIN Materia_Prima ON Taxa_Conversao.materia_prima_id = Materia_Prima.id "
                        "WHERE Taxa_Conversao.produto_id = ?;";
    vector<pair<string, double>> materialNeeds;
    if (sqlite3_prepare_v2(db, query, -1, &stmt, 0) != SQLITE_OK) {
        cerr << "Erro ao preparar query: " << sqlite3_errmsg(db) << endl;
        return materialNeeds;
    }
    sqlite3_bind_int(stmt, 1, produto_id);
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        string materialName = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        double conversionRate = sqlite3_column_double(stmt, 1);
        double requiredQuantity = round(predictedDemand * conversionRate);
        materialNeeds.push_back(make_pair(materialName, requiredQuantity));
    }
    sqlite3_finalize(stmt);
    return materialNeeds;
}

// Função para salvar os dados em um arquivo CSV
void saveToCSV(const string& demand, const vector<pair<string, double>>& materialNeeds) {
    const char* filePath = fl_file_chooser("Salvar Como", "*.csv", "");
    if (filePath) {
        ofstream outFile(filePath);
        outFile << "Produto,Demanda Prevista\n";
        outFile << "Produto," << demand << "\n";
        outFile << "\nMateriais Necessários\n";
        outFile << "Materia Prima,Quantidade\n";
        for (const auto& material : materialNeeds) {
            outFile << material.first << "," << material.second << "\n";
        }
        outFile.close();
    }
}

// Função para calcular a previsão e exibir os resultados
void calculatePrediction(Fl_Input* input, Fl_Multiline_Output* demandaBox, Fl_Multiline_Output* materialBox, Fl_Button* saveButton) {
    sqlite3* db = connectDB("production.db");
    if (!db) return;

    int produto_id = stoi(input->value());

    vector<pair<int, double>> salesData = getSalesData(db, produto_id);
    if (salesData.empty()) {
        demandaBox->value("Nenhuma venda");
        materialBox->value("Sem dados de matéria-prima");
        sqlite3_close(db);
        saveButton->deactivate();
        return;
    }

    double predictedDemand = round(linearRegression(salesData));
    string demandStr = "Demanda Prevista: " + to_string(static_cast<int>(predictedDemand));
    demandaBox->value(demandStr.c_str());

    vector<pair<string, double>> materialNeeds = calculateMaterialNeeds(db, produto_id, predictedDemand);
    string materialStr = "Materiais Necessários:\n";
    for (const auto& material : materialNeeds) {
        materialStr += material.first + ": " + to_string(static_cast<int>(material.second)) + "\n";
    }
    materialBox->value(materialStr.c_str());

    saveButton->activate();
    sqlite3_close(db);
}

// Função principal
int main() {
    Fl_Window* window = new Fl_Window(600, 550, "Previsão de Demanda e Materiais");
    Fl_Input* input = new Fl_Input(200, 60, 200, 30, "ID Produto:");
    Fl_Multiline_Output* demandaBox = new Fl_Multiline_Output(150, 120, 300, 30, "Demanda Prevista:");
    Fl_Multiline_Output* materialBox = new Fl_Multiline_Output(150, 170, 300, 200, "Materiais Necessários:");
    Fl_Button* button = new Fl_Button(200, 400, 200, 30, "Calcular");
    Fl_Button* saveButton = new Fl_Button(200, 450, 200, 30, "Salvar .CSV");

    saveButton->deactivate();

    button->callback([](Fl_Widget*, void* v) {
        Fl_Input* input = ((Fl_Input**)v)[0];
        Fl_Multiline_Output* demandaBox = ((Fl_Multiline_Output**)v)[1];
        Fl_Multiline_Output* materialBox = ((Fl_Multiline_Output**)v)[2];
        Fl_Button* saveButton = ((Fl_Button**)v)[3];
        calculatePrediction(input, demandaBox, materialBox, saveButton);
    }, new Fl_Widget* [4]{input, demandaBox, materialBox, saveButton});

    saveButton->callback([](Fl_Widget*, void* v) {
        Fl_Multiline_Output* demandaBox = ((Fl_Multiline_Output**)v)[0];
        Fl_Multiline_Output* materialBox = ((Fl_Multiline_Output**)v)[1];
        string demand = demandaBox->value();
        
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
    }, new Fl_Widget* [2]{demandaBox, materialBox});

    window->end();
    window->show();
    return Fl::run();
}
