-- Users table
CREATE TABLE IF NOT EXISTS user (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT NOT NULL,
    phone TEXT NOT NULL,
    is_registered BOOLEAN NOT NULL,
    email TEXT,
    registration_date TEXT,
    CONSTRAINT chk_registered_email CHECK (is_registered = 0 OR email IS NOT NULL),
    CONSTRAINT chk_registered_date CHECK (is_registered = 0 OR registration_date IS NOT NULL)
);
CREATE UNIQUE INDEX IF NOT EXISTS idx_user_phone ON user(phone);
CREATE UNIQUE INDEX IF NOT EXISTS idx_user_email ON user(email) WHERE email IS NOT NULL;

-- Products table
CREATE TABLE IF NOT EXISTS product (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT NOT NULL,
    description TEXT,
    price REAL NOT NULL,
    type TEXT NOT NULL,
    image BLOB,
    size TEXT,
    is_custom BOOLEAN NOT NULL,
    CONSTRAINT chk_price_positive CHECK (price > 0),
    CONSTRAINT chk_size CHECK (size IN ('Small', 'Medium', 'Large', 'Extra Large') OR size IS NULL)
);

-- Orders table
CREATE TABLE IF NOT EXISTS "order" (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    user_id INTEGER NOT NULL,
    order_time TEXT NOT NULL,
    total_price REAL NOT NULL,
    delivery_method TEXT NOT NULL,
    delivery_address TEXT,
    current_status TEXT NOT NULL,
    status_updated_at TEXT NOT NULL,
    item_count INTEGER NOT NULL,
    FOREIGN KEY (user_id) REFERENCES user (id) ON DELETE RESTRICT ON UPDATE CASCADE,
    CONSTRAINT chk_total_price_positive CHECK (total_price > 0),
    CONSTRAINT chk_item_count_positive CHECK (item_count > 0),
    CONSTRAINT chk_order_status CHECK (
        current_status IN ('Preparing', 'In Transit', 'Delivered', 'Completed', 'Cancelled')
    ),
    CONSTRAINT chk_delivery_method CHECK (
        delivery_method IN ('Delivery', 'Pickup')
    ),
    CONSTRAINT chk_delivery_address CHECK (
        (delivery_method = 'Pickup' AND delivery_address IS NULL) OR
        (delivery_method = 'Delivery' AND delivery_address IS NOT NULL)
    )
);

-- Order items table
CREATE TABLE IF NOT EXISTS order_item (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    order_id INTEGER NOT NULL,
    product_id INTEGER NOT NULL,
    quantity INTEGER NOT NULL,
    price REAL NOT NULL,
    FOREIGN KEY (order_id) REFERENCES "order" (id) ON DELETE CASCADE ON UPDATE CASCADE,
    FOREIGN KEY (product_id) REFERENCES product (id) ON DELETE RESTRICT ON UPDATE CASCADE,
    CONSTRAINT chk_quantity_positive CHECK (quantity > 0),
    CONSTRAINT chk_item_price_positive CHECK (price > 0)
);

-- Promotions table
CREATE TABLE IF NOT EXISTS promotion (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT NOT NULL,
    description TEXT,
    start_date TEXT NOT NULL,
    end_date TEXT NOT NULL,
    discount_percent REAL NOT NULL,
    is_active BOOLEAN NOT NULL,
    product_id INTEGER NOT NULL,
    FOREIGN KEY (product_id) REFERENCES product (id) ON DELETE CASCADE ON UPDATE CASCADE,
    CONSTRAINT chk_discount_range CHECK (discount_percent > 0 AND discount_percent <= 100),
    CONSTRAINT chk_date_range CHECK (start_date <= end_date)
);

-- Toppings table
CREATE TABLE IF NOT EXISTS topping (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT NOT NULL,
    price REAL NOT NULL,
    category TEXT NOT NULL,
    CONSTRAINT chk_topping_price_positive CHECK (price > 0),
    CONSTRAINT chk_category CHECK (
        category IN ('Cheese', 'Meat', 'Vegetables', 'Fruits', 'Sauce', 'Spices')
    )
);
CREATE UNIQUE INDEX IF NOT EXISTS idx_topping_name ON topping(name);

-- Product toppings junction table
CREATE TABLE IF NOT EXISTS product_topping (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    product_id INTEGER NOT NULL,
    topping_id INTEGER NOT NULL,
    is_default BOOLEAN NOT NULL,
    FOREIGN KEY (product_id) REFERENCES product (id) ON DELETE CASCADE ON UPDATE CASCADE,
    FOREIGN KEY (topping_id) REFERENCES topping (id) ON DELETE RESTRICT ON UPDATE CASCADE,
    CONSTRAINT unique_product_topping UNIQUE (product_id, topping_id)
);

-- Data insertion statements
-- Note: Without explicit ID values, SQLite will automatically assign them
INSERT INTO user (name, phone, is_registered, email, registration_date)
VALUES 
('Alexander Kovalchuk', '+380501234567', 1, 'alexander.k@email.com', '2023-01-15'),
('Maria Shevchenko', '+380672345678', 1, 'maria.shev@email.com', '2023-02-10'),
('Andriy Bondarenko', '+380633456789', 1, 'andriy.b@email.com', '2023-03-05'),
('Oksana Kravchenko', '+380504567890', 1, 'oksana.k@email.com', '2023-04-20'),
('Mykola Petrenko', '+380675678901', 0, NULL, NULL),
('Sofia Melnyk', '+380636789012', 1, 'sofia.m@email.com', '2023-06-12'),
('Dmytro Lysenko', '+380507890123', 1, 'dmytro.l@email.com', '2023-07-25'),
('Yulia Tkachenko', '+380678901234', 0, NULL, NULL),
('Taras Moroz', '+380639012345', 1, 'taras.m@email.com', '2023-09-08'),
('Iryna Bilous', '+380500123456', 1, 'iryna.b@email.com', '2023-10-17');

INSERT INTO product (name, description, price, type, image, size, is_custom)
VALUES 
('Margherita Pizza', 'Classic pizza with tomato sauce, mozzarella, and basil', 179.99, 'Pizza', NULL, 'Medium', 0),
('Pepperoni Pizza', 'Traditional pizza with pepperoni', 219.99, 'Pizza', NULL, 'Medium', 0),
('Vegetable Pizza', 'Pizza loaded with fresh vegetables', 239.99, 'Pizza', NULL, 'Large', 0),
('BBQ Chicken Pizza', 'Pizza with BBQ sauce, chicken, and red onions', 249.99, 'Pizza', NULL, 'Medium', 0),
('Hawaiian Pizza', 'Pizza with ham and pineapple', 229.99, 'Pizza', NULL, 'Medium', 0),
('Four Cheese Pizza', 'Pizza with four types of cheese: mozzarella, parmesan, gorgonzola, feta', 259.99, 'Pizza', NULL, 'Medium', 0),
('Carbonara Pizza', 'Pizza with bacon, egg, and cream sauce', 249.99, 'Pizza', NULL, 'Medium', 0),
('Meat Lovers Pizza', 'Pizza with various types of meat: pepperoni, ham, bacon, chicken', 269.99, 'Pizza', NULL, 'Large', 0),
('Custom Pizza', 'Create your own pizza with your choice of toppings', 199.99, 'Pizza', NULL, 'Medium', 1),
('Mushroom Pizza', 'Pizza with various types of mushrooms and truffle oil', 239.99, 'Pizza', NULL, 'Medium', 0),
('Diablo Pizza', 'Spicy pizza with pepperoni, jalapeños, and hot sauce', 229.99, 'Pizza', NULL, 'Medium', 0),
('Capricciosa Pizza', 'Pizza with ham, mushrooms, artichokes, and olives', 249.99, 'Pizza', NULL, 'Medium', 0);

INSERT INTO "order" (user_id, order_time, total_price, delivery_method, delivery_address, current_status, status_updated_at, item_count)
VALUES 
(1, '2025-04-01 18:30:00', 919.96, 'Delivery', '12 Main St, Kyiv', 'Delivered', '2025-04-01 19:45:00', 3),
(2, '2025-04-02 12:15:00', 449.98, 'Pickup', NULL, 'Completed', '2025-04-02 12:45:00', 2),
(3, '2025-04-03 19:00:00', 1239.95, 'Delivery', '45 Victory Ave, Kyiv', 'Delivered', '2025-04-03 20:10:00', 4),
(5, '2025-04-04 17:45:00', 509.98, 'Pickup', NULL, 'Completed', '2025-04-04 18:20:00', 2),
(6, '2025-04-05 20:00:00', 699.97, 'Delivery', '23 Harbor St, Kyiv', 'Delivered', '2025-04-05 21:15:00', 3),
(1, '2025-04-06 19:30:00', 759.97, 'Delivery', '12 Main St, Kyiv', 'Delivered', '2025-04-06 20:45:00', 2),
(4, '2025-04-08 18:00:00', 709.97, 'Pickup', NULL, 'Completed', '2025-04-08 18:30:00', 3),
(7, '2025-04-09 12:30:00', 429.98, 'Delivery', '17 Franko St, Kyiv', 'In Transit', '2025-04-09 13:00:00', 2),
(9, '2025-04-10 19:15:00', 1249.95, 'Delivery', '34 Lesya Ukrainka Blvd, Kyiv', 'In Transit', '2025-04-10 19:30:00', 4),
(10, '2025-04-11 17:30:00', 989.96, 'Pickup', NULL, 'Preparing', '2025-04-11 17:35:00', 3);

-- For order_item, product_id and order_id will reference the auto-generated IDs
-- Assuming the order of insertion matches the original IDs
INSERT INTO order_item (order_id, product_id, quantity, price)
VALUES 
(1, 1, 1, 179.99),
(1, 6, 1, 259.99),
(1, 10, 2, 479.98),
(2, 2, 1, 219.99),
(2, 5, 1, 229.99),
(3, 3, 1, 239.99),
(3, 7, 1, 249.99),
(3, 8, 1, 269.99),
(3, 10, 2, 479.98),
(4, 4, 1, 249.99),
(4, 6, 1, 259.99),
(5, 9, 1, 199.99),
(5, 7, 1, 249.99),
(5, 12, 1, 249.99),
(6, 2, 1, 219.99),
(6, 8, 2, 539.98),
(7, 11, 1, 229.99),
(7, 5, 1, 229.99),
(7, 4, 1, 249.99),
(8, 1, 1, 179.99),
(8, 12, 1, 249.99),
(9, 3, 1, 239.99),
(9, 4, 1, 249.99),
(9, 6, 2, 519.98),
(9, 10, 1, 239.99),
(10, 9, 1, 199.99),
(10, 7, 1, 249.99),
(10, 8, 2, 539.98);

INSERT INTO promotion (name, description, start_date, end_date, discount_percent, is_active, product_id)
VALUES 
('April Madness', '20% off all pizzas for April Madness', '2025-04-01', '2025-04-30', 20.0, 1, 1),
('BBQ Special', '15% off BBQ Chicken Pizza', '2025-04-15', '2025-05-15', 15.0, 1, 4),
('Cheese Wednesday', '25% off Four Cheese Pizza every Wednesday', '2025-01-01', '2025-12-31', 25.0, 1, 6),
('Diablo Days', '10% off Diablo Pizza on Mondays', '2025-04-01', '2025-05-31', 10.0, 1, 11),
('Margherita Mania', '15% off Margherita Pizza', '2025-04-15', '2025-06-15', 15.0, 1, 1),
('Meat Month', '20% off Meat Lovers Pizza', '2025-05-01', '2025-05-31', 20.0, 0, 8),
('Pepperoni Promo', '15% off Pepperoni Pizza', '2025-04-01', '2025-04-30', 15.0, 1, 2),
('Family Weekend', '10% off large pizzas on weekends', '2025-04-01', '2025-07-31', 10.0, 1, 3);

INSERT INTO topping (name, price, category)
VALUES 
('Extra Mozzarella', 29.99, 'Cheese'),
('Parmesan', 32.99, 'Cheese'),
('Feta', 32.99, 'Cheese'),
('Pepperoni', 34.99, 'Meat'),
('Ham', 34.99, 'Meat'),
('Chicken', 34.99, 'Meat'),
('Bacon', 36.99, 'Meat'),
('Mushrooms', 24.99, 'Vegetables'),
('Bell Peppers', 22.99, 'Vegetables'),
('Onions', 18.99, 'Vegetables'),
('Olives', 26.99, 'Vegetables'),
('Pineapple', 29.99, 'Fruits');

-- For product_topping, assuming product and topping IDs match the order of insertion
INSERT INTO product_topping (product_id, topping_id, is_default)
VALUES 
(1, 1, 1),
(2, 1, 1),
(2, 4, 1),
(3, 1, 1),
(3, 8, 1),
(3, 9, 1),
(3, 10, 1),
(3, 11, 1),
(4, 1, 1),
(4, 6, 1),
(4, 10, 1),
(5, 1, 1),
(5, 5, 1),
(5, 12, 1),
(6, 1, 1),
(6, 2, 1),
(6, 3, 1),
(7, 1, 1),
(7, 7, 1),
(8, 1, 1),
(8, 4, 1),
(8, 5, 1),
(8, 6, 1),
(8, 7, 1),
(9, 1, 1),
(9, 2, 0),
(9, 3, 0),
(9, 4, 0),
(9, 5, 0),
(9, 6, 0),
(9, 7, 0),
(9, 8, 0),
(9, 9, 0),
(9, 10, 0),
(9, 11, 0),
(9, 12, 0),
(10, 1, 1),
(10, 8, 1),
(11, 1, 1),
(11, 4, 1),
(11, 9, 1),
(12, 1, 1),
(12, 5, 1),
(12, 8, 1),
(12, 11, 1);

-- Create indexes for performance
CREATE INDEX IF NOT EXISTS idx_order_user_id ON "order"(user_id);
CREATE INDEX IF NOT EXISTS idx_order_status ON "order"(current_status);
CREATE INDEX IF NOT EXISTS idx_order_item_order_id ON order_item(order_id);
CREATE INDEX IF NOT EXISTS idx_order_item_product_id ON order_item(product_id);
CREATE INDEX IF NOT EXISTS idx_promotion_product_id ON promotion(product_id);
CREATE INDEX IF NOT EXISTS idx_promotion_active ON promotion(is_active);
CREATE INDEX IF NOT EXISTS idx_product_topping_product_id ON product_topping(product_id);
CREATE INDEX IF NOT EXISTS idx_product_topping_topping_id ON product_topping(topping_id);
CREATE INDEX IF NOT EXISTS idx_product_type ON product(type);