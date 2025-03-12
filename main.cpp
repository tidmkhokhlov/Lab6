#include <iostream>
#include <libpq-fe.h>
#include <string>
#include <limits>
#include <windows.h>
#include <vector>
#include <sstream>

// Исполнение запроса
void executeSQL(PGconn *conn, const std::string &sql) {
    PGresult *res = PQexec(conn, sql.c_str());
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        std::cerr << "SQL execution error: " << PQerrorMessage(conn) << std::endl;
        PQclear(res);
        return;
    }
    PQclear(res);
}

// Вывод результата в консоль
void printResults(PGresult *res) {
    int rows = PQntuples(res);
    int cols = PQnfields(res);

    for (int i = 0; i < cols; ++i) {
        std::cout << PQfname(res, i) << "\t";
    }
    std::cout << std::endl;

    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            std::cout << PQgetvalue(res, i, j) << "\t";
        }
        std::cout << std::endl;
    }
}

// Функция создания таблицы
void createTable(PGconn *conn) {
    std::string sql = R"(
        -- Создание таблицы book
        CREATE TABLE IF NOT EXISTS book (
            id SERIAL PRIMARY KEY,
            title VARCHAR(255) NOT NULL,
            author VARCHAR(255) NOT NULL,
            year INT NOT NULL
        );

        -- Очистка таблицы
        CREATE OR REPLACE PROCEDURE ClearTable()
        LANGUAGE plpgsql
        AS $$
        BEGIN
            DELETE FROM book;
        END;
        $$;

        -- Добавление записи
        CREATE OR REPLACE PROCEDURE AddBook(p_title VARCHAR(255), p_author VARCHAR(255), p_year INT)
        LANGUAGE plpgsql
        AS $$
        BEGIN
            INSERT INTO book (title, author, year) VALUES (p_title, p_author, p_year);
        END;
        $$;

        -- Поиск книги по названию
        CREATE OR REPLACE FUNCTION FindBookByTitle(p_title VARCHAR(255))
        RETURNS TABLE(id INT, title VARCHAR(255), author VARCHAR(255), year INT)
        LANGUAGE plpgsql
        AS $$
        BEGIN
            RETURN QUERY SELECT * FROM book WHERE book.title = p_title;
        END;
        $$;

        -- Поиск книги по автору
        CREATE OR REPLACE FUNCTION FindBookByAuthor(p_author VARCHAR(255))
        RETURNS TABLE(id INT, title VARCHAR(255), author VARCHAR(255), year INT)
        LANGUAGE plpgsql
        AS $$
        BEGIN
            RETURN QUERY SELECT * FROM book WHERE book.author = p_author;
        END;
        $$;

        -- Поиск книги по году
        CREATE OR REPLACE FUNCTION FindBookByYear(p_year INT)
        RETURNS TABLE(id INT, title VARCHAR(255), author VARCHAR(255), year INT)
        LANGUAGE plpgsql
        AS $$
        BEGIN
            RETURN QUERY SELECT * FROM book WHERE book.year = p_year;
        END;
        $$;

        -- Обновление книги
        CREATE OR REPLACE PROCEDURE UpdateBook(p_id INT, p_title VARCHAR(255), p_author VARCHAR(255), p_year INT)
        LANGUAGE plpgsql
        AS $$
        BEGIN
            UPDATE book SET title = p_title, author = p_author, year = p_year WHERE id = p_id;
        END;
        $$;

        -- Удаление книги по названию
        CREATE OR REPLACE PROCEDURE DeleteBookByTitle(p_title VARCHAR(255))
        LANGUAGE plpgsql
        AS $$
        BEGIN
            DELETE FROM book WHERE book.title = p_title;
        END;
        $$;

        -- Вывод всего содержимого таблицы
        CREATE OR REPLACE FUNCTION ViewAllRecords()
        RETURNS TABLE(id INT, title VARCHAR(255), author VARCHAR(255), year INT)
        LANGUAGE plpgsql
        AS $$
        BEGIN
            RETURN QUERY SELECT * FROM book ORDER BY id;
        END;
        $$;

        -- Предоставление прав администратору
        ALTER ROLE admin WITH SUPERUSER; -- Администратор имеет все права
        GRANT ALL PRIVILEGES ON DATABASE library TO admin; -- Полный доступ к базе данных
        GRANT ALL PRIVILEGES ON TABLE book TO admin; -- Полный доступ к таблице book
        GRANT EXECUTE ON PROCEDURE ClearTable() TO admin; -- Доступ к процедуре очистки таблицы
        GRANT EXECUTE ON PROCEDURE AddBook(VARCHAR, VARCHAR, INT) TO admin; -- Доступ к процедуре добавления книги
        GRANT EXECUTE ON FUNCTION FindBookByTitle(VARCHAR) TO admin; -- Доступ к функции поиска по названию
        GRANT EXECUTE ON FUNCTION FindBookByAuthor(VARCHAR) TO admin; -- Доступ к функции поиска по автору
        GRANT EXECUTE ON FUNCTION FindBookByYear(INT) TO admin; -- Доступ к функции поиска по году
        GRANT EXECUTE ON PROCEDURE UpdateBook(INT, VARCHAR, VARCHAR, INT) TO admin; -- Доступ к процедуре обновления книги
        GRANT EXECUTE ON PROCEDURE DeleteBookByTitle(VARCHAR) TO admin; -- Доступ к процедуре удаления книги по названию
        GRANT EXECUTE ON FUNCTION ViewAllRecords() TO admin; -- Доступ к функции просмотра всех записей

        -- Предоставление прав гостю
        GRANT SELECT ON TABLE book TO guest; -- Доступ только для чтения таблицы book
        GRANT EXECUTE ON FUNCTION FindBookByTitle(VARCHAR) TO guest; -- Доступ к функции поиска по названию
        GRANT EXECUTE ON FUNCTION FindBookByAuthor(VARCHAR) TO guest; -- Доступ к функции поиска по автору
        GRANT EXECUTE ON FUNCTION FindBookByYear(INT) TO guest; -- Доступ к функции поиска по году
        GRANT EXECUTE ON FUNCTION ViewAllRecords() TO guest; -- Доступ к функции просмотра всех записей
    )";
    executeSQL(conn, sql);
    std::cout << "Table 'book' created or already exists." << std::endl;
}

void createRoles(PGconn *conn) {
    std::string roles = R"(
        -- Удаление роли admin, если она существует
        DROP ROLE IF EXISTS admin;

        -- Создание роли admin
        CREATE ROLE admin WITH LOGIN PASSWORD 'admin_password';

        -- Удаление роли guest, если она существует
        DROP ROLE IF EXISTS guest;

        -- Создание роли guest
        CREATE ROLE guest WITH LOGIN PASSWORD 'guest_password';

        -- Предоставление прав администратору
        ALTER ROLE admin WITH SUPERUSER; -- Администратор имеет все права
        GRANT ALL PRIVILEGES ON DATABASE library TO admin; -- Полный доступ к базе данных
        GRANT ALL PRIVILEGES ON TABLE book TO admin; -- Полный доступ к таблице book
        GRANT EXECUTE ON PROCEDURE ClearTable() TO admin; -- Доступ к процедуре очистки таблицы
        GRANT EXECUTE ON PROCEDURE AddBook(VARCHAR, VARCHAR, INT) TO admin; -- Доступ к процедуре добавления книги
        GRANT EXECUTE ON FUNCTION FindBookByTitle(VARCHAR) TO admin; -- Доступ к функции поиска по названию
        GRANT EXECUTE ON FUNCTION FindBookByAuthor(VARCHAR) TO admin; -- Доступ к функции поиска по автору
        GRANT EXECUTE ON FUNCTION FindBookByYear(INT) TO admin; -- Доступ к функции поиска по году
        GRANT EXECUTE ON PROCEDURE UpdateBook(INT, VARCHAR, VARCHAR, INT) TO admin; -- Доступ к процедуре обновления книги
        GRANT EXECUTE ON PROCEDURE DeleteBookByTitle(VARCHAR) TO admin; -- Доступ к процедуре удаления книги по названию
        GRANT EXECUTE ON FUNCTION ViewAllRecords() TO admin; -- Доступ к функции просмотра всех записей

        -- Предоставление прав гостю
        GRANT SELECT ON TABLE book TO guest; -- Доступ только для чтения таблицы book
        GRANT EXECUTE ON FUNCTION FindBookByTitle(VARCHAR) TO guest; -- Доступ к функции поиска по названию
        GRANT EXECUTE ON FUNCTION FindBookByAuthor(VARCHAR) TO guest; -- Доступ к функции поиска по автору
        GRANT EXECUTE ON FUNCTION FindBookByYear(INT) TO guest; -- Доступ к функции поиска по году
        GRANT EXECUTE ON FUNCTION ViewAllRecords() TO guest; -- Доступ к функции просмотра всех записей
    )";
    executeSQL(conn, roles);
}

// Функция очистки таблицы
void clearTable(PGconn *conn) {
    std::string sql = "CALL ClearTable();";
    executeSQL(conn, sql);
    std::cout << "Table 'book' cleared." << std::endl;
}

// Функция добавления
void addBook(PGconn *conn, const std::string &title, const std::string &author, int year) {
    std::string sql = "CALL AddBook('" + title + "', '" + author + "', " + std::to_string(year) + ");";
    executeSQL(conn, sql);
    std::cout << "Book added successfully." << std::endl;
}

// Поиск по названию
void findBookByTitle(PGconn *conn, const std::string &title) {
    std::string sql = "SELECT * FROM FindBookByTitle('" + title + "');";
    PGresult *res = PQexec(conn, sql.c_str());
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        std::cerr << "SQL execution error: " << PQerrorMessage(conn) << std::endl;
        PQclear(res);
        return;
    }
    printResults(res);
    PQclear(res);
}

// Поиск по автору
void findBookByAuthor(PGconn *conn, const std::string &author) {
    std::string sql = "SELECT * FROM FindBookByAuthor('" + author + "');";
    PGresult *res = PQexec(conn, sql.c_str());
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        std::cerr << "SQL execution error: " << PQerrorMessage(conn) << std::endl;
        PQclear(res);
        return;
    }
    printResults(res);
    PQclear(res);
}

// Поиск по году
void findBookByYear(PGconn *conn, int year) {
    std::string sql = "SELECT * FROM FindBookByYear(" + std::to_string(year) + ");";
    PGresult *res = PQexec(conn, sql.c_str());
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        std::cerr << "SQL execution error: " << PQerrorMessage(conn) << std::endl;
        PQclear(res);
        return;
    }
    printResults(res);
    PQclear(res);
}

// Функция обновления
void updateBook(PGconn *conn, int id, const std::string &title, const std::string &author, int year) {
    std::string sql = "CALL UpdateBook(" + std::to_string(id) + ", '" + title + "', '" + author + "', " + std::to_string(year) + ");";
    executeSQL(conn, sql);
    std::cout << "Book updated successfully." << std::endl;
}

// Функция удаления
void deleteBookByTitle(PGconn *conn, const std::string &title) {
    std::string sql = "CALL DeleteBookByTitle('" + title + "');";
    executeSQL(conn, sql);
    std::cout << "Book deleted successfully." << std::endl;
}

// Функция вывода всех записей
void viewAllRecords(PGconn *conn) {
    std::string sql = "SELECT * FROM ViewAllRecords();";
    PGresult *res = PQexec(conn, sql.c_str());
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        std::cerr << "SQL execution error: " << PQerrorMessage(conn) << std::endl;
        PQclear(res);
        return;
    }
    printResults(res);
    PQclear(res);
}

// Функция Создания роли
void createRole(PGconn *conn) {
    std::string roleName, rolePassword;
    std::cout << "Enter role name: ";
    std::getline(std::cin, roleName);
    std::cout << "Enter role password: ";
    std::getline(std::cin, rolePassword);

    // Создание роли
    std::string createRoleSQL = "CREATE ROLE " + roleName + " WITH LOGIN PASSWORD '" + rolePassword + "';";
    executeSQL(conn, createRoleSQL);
    std::cout << "Role '" << roleName << "' created.\n";

    // Список доступных функций
    std::vector<std::string> functions = {
            "AddBook", "FindBookByTitle", "FindBookByAuthor", "FindBookByYear",
            "UpdateBook", "DeleteBookByTitle", "ViewAllRecords"
    };

    // Выбор доступных функций
    std::vector<std::string> selectedFunctions;
    std::cout << "Select functions to grant to the role:\n";
    for (size_t i = 0; i < functions.size(); ++i) {
        std::cout << (i + 1) << ". " << functions[i] << "\n";
    }
    std::cout << "Enter the numbers of the functions (comma-separated): ";
    std::string input;
    std::getline(std::cin, input);

    // Разделение ввода на номера функций
    std::stringstream ss(input);
    std::string token;
    while (std::getline(ss, token, ',')) {
        int choice = std::stoi(token);
        if (choice >= 1 && choice <= functions.size()) {
            selectedFunctions.push_back(functions[choice - 1]);
        }
    }

    // Назначение прав на выбранные функции
    for (const auto &func : selectedFunctions) {
        std::string grantSQL;
        if (func == "AddBook") {
            grantSQL = "GRANT EXECUTE ON PROCEDURE AddBook(VARCHAR, VARCHAR, INT) TO " + roleName + ";";
        } else if (func == "FindBookByTitle") {
            grantSQL = "GRANT EXECUTE ON FUNCTION FindBookByTitle(VARCHAR) TO " + roleName + ";";
        } else if (func == "FindBookByAuthor") {
            grantSQL = "GRANT EXECUTE ON FUNCTION FindBookByAuthor(VARCHAR) TO " + roleName + ";";
        } else if (func == "FindBookByYear") {
            grantSQL = "GRANT EXECUTE ON FUNCTION FindBookByYear(INT) TO " + roleName + ";";
        } else if (func == "UpdateBook") {
            grantSQL = "GRANT EXECUTE ON PROCEDURE UpdateBook(INT, VARCHAR, VARCHAR, INT) TO " + roleName + ";";
        } else if (func == "DeleteBookByTitle") {
            grantSQL = "GRANT EXECUTE ON PROCEDURE DeleteBookByTitle(VARCHAR) TO " + roleName + ";";
        } else if (func == "ViewAllRecords") {
            grantSQL = "GRANT EXECUTE ON FUNCTION ViewAllRecords() TO " + roleName + ";";
        }

        if (!grantSQL.empty()) {
            executeSQL(conn, grantSQL);
            std::cout << "Granted " << func << " to role '" << roleName << "'.\n";
        }
    }

    std::cout << "Role '" << roleName << "' created with selected permissions.\n";
}

int getValidYear() {
    int year;
    while (true) {
        std::cout << "Enter publication year: ";
        std::cin >> year;
        if (std::cin.fail()) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Clear input buffer
            std::cout << "Invalid input. Please enter a valid year (numbers only).\n";
        } else {
            std::cin.ignore();
            return year;
        }
    }
}

int main() {
    SetConsoleOutputCP(CP_UTF8);
//    const char *conninfopostgres = "dbname=library user=postgres password=111 hostaddr=127.0.0.1 port=5432";
//    PGconn *connpostgres = PQconnectdb(conninfopostgres);
//    createRoles(connpostgres);

    std::string username, password;

    // Запрос имени пользователя и пароля
    std::cout << "Enter username: ";
    std::getline(std::cin, username);
    std::cout << "Enter password: ";
    std::getline(std::cin, password);

    // Формирование строки подключения
    std::string conninfo = "dbname=postgres user=" + username + " password=" + password + " hostaddr=127.0.0.1 port=5432";
    PGconn *conn = PQconnectdb(conninfo.c_str());

    if (PQstatus(conn) != CONNECTION_OK) {
        std::cerr << "Connection to database failed: " << PQerrorMessage(conn) << std::endl;
        PQfinish(conn);
        return 1;
    }

    std::string conninfolib = "dbname=library user=" + username + " password=" + password + " hostaddr=127.0.0.1 port=5432";
    PGconn *connlib = PQconnectdb(conninfolib.c_str());

    if (PQstatus(connlib) != CONNECTION_OK) {
        std::cerr << "Connection to 'library' database failed: " << PQerrorMessage(connlib) << std::endl;
        PQfinish(conn);
        return 1;
    }

    int choice;
    std::string input;
    int id, year;
    std::string title, author;

    while (true) {
        std::cout << "\nMenu:\n";
        std::cout << "1. Create Database\n";
        std::cout << "2. Delete Database\n";
        std::cout << "3. Clear Table\n";
        std::cout << "4. Add Record\n";
        std::cout << "5. Search Record\n";
        std::cout << "6. Update Record\n";
        std::cout << "7. Delete Record\n";
        std::cout << "8. View All Records\n";
        std::cout << "9. Create Role\n";
        std::cout << "0. Exit\n";
        std::cout << "Choose an option: ";
        std::cin >> choice;
        std::cin.ignore();

        switch (choice) {
            case 1:
                executeSQL(conn, "CREATE DATABASE library;");
                std::cout << "Database 'library' created or already exists.\n";
                connlib = PQconnectdb(conninfolib.c_str());
                createTable(connlib);
                break;

            case 2:
                PQfinish(connlib);
                executeSQL(conn, "DROP DATABASE IF EXISTS library;");
                std::cout << "Database 'library' deleted.\n";
                break;

            case 3:
                clearTable(connlib);
                break;

            case 4:
                std::cout << "Enter book title: ";
                std::getline(std::cin, title);
                std::cout << "Enter book author: ";
                std::getline(std::cin, author);
                year = getValidYear();
                addBook(connlib, title, author, year);
                break;

            case 5:
                std::cout << "Search by:\n";
                std::cout << "1. Title\n";
                std::cout << "2. Author\n";
                std::cout << "3. Year\n";
                std::cout << "Choose an option: ";
                std::cin >> choice;
                std::cin.ignore();

                switch (choice) {
                    case 1:
                        std::cout << "Enter book title: ";
                        std::getline(std::cin, title);
                        findBookByTitle(connlib, title);
                        break;

                    case 2:
                        std::cout << "Enter book author: ";
                        std::getline(std::cin, author);
                        findBookByAuthor(connlib, author);
                        break;

                    case 3:
                        year = getValidYear();
                        findBookByYear(connlib, year);
                        break;

                    default:
                        std::cout << "Invalid choice.\n";
                        break;
                }
                break;

            case 6:
                std::cout << "Enter book ID to update: ";
                std::cin >> id;
                std::cin.ignore(); // Clear input buffer
                std::cout << "Enter new book title: ";
                std::getline(std::cin, title);
                std::cout << "Enter new book author: ";
                std::getline(std::cin, author);
                year = getValidYear(); // Validate year input
                updateBook(connlib, id, title, author, year);
                break;

            case 7:
                std::cout << "Enter book title to delete: ";
                std::getline(std::cin, title);
                deleteBookByTitle(connlib, title);
                break;

            case 8:
                std::cout << "All records in the table:\n";
                viewAllRecords(connlib);
                break;

            case 9:
                createRole(conn);
                break;

            case 0:
                // Проверка, открыто ли подключение connlib
                if (PQstatus(connlib) != CONNECTION_BAD) {
                    PQfinish(connlib);
                    std::cout << "Connection to 'library' database closed.\n";
                }

                // Закрытие подключения conn
                PQfinish(conn);
                std::cout << "Connection to 'postgres' database closed.\n";

                std::cout << "Exiting the program.\n";
                return 0;

            default:
                std::cout << "Invalid choice. Please try again.\n";
                break;
        }
    }

    PQfinish(conn);
    PQfinish(connlib);
    return 0;
}