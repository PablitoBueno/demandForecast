import sqlite3

# Connect to the database (creates the file if it does not exist)
conn = sqlite3.connect('production.db')
cursor = conn.cursor()

# Create tables for products, sales, and raw materials
cursor.execute('''CREATE TABLE IF NOT EXISTS Product (
                    id INTEGER PRIMARY KEY,
                    name TEXT
                  )''')

cursor.execute('''CREATE TABLE IF NOT EXISTS Sales (
                    id INTEGER PRIMARY KEY,
                    product_id INTEGER,
                    date TEXT,
                    sales REAL,
                    FOREIGN KEY(product_id) REFERENCES Product(id)
                  )''')

cursor.execute('''CREATE TABLE IF NOT EXISTS Raw_Material (
                    id INTEGER PRIMARY KEY,
                    name TEXT
                  )''')

cursor.execute('''CREATE TABLE IF NOT EXISTS Conversion_Rate (
                    id INTEGER PRIMARY KEY,
                    product_id INTEGER,
                    raw_material_id INTEGER,
                    quantity_needed REAL,
                    FOREIGN KEY(product_id) REFERENCES Product(id),
                    FOREIGN KEY(raw_material_id) REFERENCES Raw_Material(id)
                  )''')

# Insert products
products = [("Cake",), ("Bread",), ("Cookie",)]
cursor.executemany("INSERT INTO Product (name) VALUES (?)", products)

# Insert raw materials
raw_materials = [
    ("Wheat Flour",), ("Sugar",), ("Yeast",),
    ("Eggs",), ("Powdered Milk",), ("Butter",)
]
cursor.executemany("INSERT INTO Raw_Material (name) VALUES (?)", raw_materials)

# Insert conversion rates (amount of raw material required per product unit)
conversion_rates = [
    # Rates for Cake
    (1, 1, 200),  # Wheat Flour for Cake: 200g
    (1, 2, 100),  # Sugar for Cake: 100g
    (1, 3, 10),   # Yeast for Cake: 10g
    (1, 4, 3),    # Eggs for Cake: 3 units
    (1, 5, 50),   # Powdered Milk for Cake: 50g
    (1, 6, 80),   # Butter for Cake: 80g

    # Rates for Bread
    (2, 1, 300),  # Wheat Flour for Bread: 300g
    (2, 2, 20),   # Sugar for Bread: 20g
    (2, 3, 5),    # Yeast for Bread: 5g
    (2, 6, 30),   # Butter for Bread: 30g

    # Rates for Cookie
    (3, 1, 150),  # Wheat Flour for Cookie: 150g
    (3, 2, 50),   # Sugar for Cookie: 50g
    (3, 5, 30),   # Powdered Milk for Cookie: 30g
    (3, 6, 50)    # Butter for Cookie: 50g
]
cursor.executemany("INSERT INTO Conversion_Rate (product_id, raw_material_id, quantity_needed) VALUES (?, ?, ?)", conversion_rates)

# Insert sales data (example records for Cake, Bread, and Cookie)
sales = [
    # Sales for Cake
    (1, "20240101", 20),
    (1, "20240102", 25),
    (1, "20240103", 30),

    # Sales for Bread
    (2, "20240101", 50),
    (2, "20240102", 55),
    (2, "20240103", 60),

    # Sales for Cookie
    (3, "20240101", 40),
    (3, "20240102", 45),
    (3, "20240103", 50)
]
cursor.executemany("INSERT INTO Sales (product_id, date, sales) VALUES (?, ?, ?)", sales)

# Save changes and close the connection
conn.commit()
conn.close()

print("Database populated successfully.")
