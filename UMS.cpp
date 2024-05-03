#include <iostream>
#include <vector>
#include <string>
#include <type_traits>
#include <algorithm> // For std::find
#include <memory> // For unique_ptr
#include <mysql/mysql.h> // MySQL C++ Connector

using namespace std;

class User {
protected:
    int id;
    string username;
    string password;
    string role;
    MYSQL* connection; // MySQL connection object

    // Function to establish database connection
    void connectToDatabase() {
        connection = mysql_init(NULL);
        if (connection == nullptr) {
            cerr << "Error initializing MySQL: " << mysql_error(connection) << endl;
            exit(1);
        }
        if (!mysql_real_connect(connection, "localhost", "root", "thesun", "Falcon", 0, NULL, 0)) {
            cerr << "Error connecting to MySQL: " << mysql_error(connection) << endl;
            exit(1);
        }
    }
public:
    // Constructor
    User(int _id, string _username, string _password, string _role)
        : id(_id), username(_username), password(_password), role(_role), connection(nullptr) {}

    // Destructor (made virtual)
    virtual ~User() {
        if (connection != nullptr) {
            mysql_close(connection);
        }
    }   

    // Method to authenticate user by comparing provided password with database
    bool authenticate() {
        if (connection == nullptr) {
            connectToDatabase();
        }
        string query = "SELECT * FROM users WHERE username='" + username + "' AND password='" + password + "'";
        if (mysql_query(connection, query.c_str()) != 0) {
            cerr << "Error authenticating user: " << mysql_error(connection) << endl;
            exit(1);
        }
        MYSQL_RES* result = mysql_store_result(connection);
        if (result == nullptr) {
            cerr << "Error fetching result: " << mysql_error(connection) << endl;
            exit(1);
        }
        bool authenticated = (mysql_num_rows(result) == 1);
        mysql_free_result(result);
        return authenticated;
    }

    // Getter method to retrieve user role
    string getRole() const {
        return role;
    }

    // Method to register user in the database
    void registerUser() {
        if (connection == nullptr) {
            connectToDatabase();
        }
        string query = "INSERT INTO users (username, password, role) VALUES ('" + username + "', '" + password + "', '" + role + "')";
        if (mysql_query(connection, query.c_str()) != 0) {
            cerr << "Error registering user: " << mysql_error(connection) << endl;
            exit(1);
        }
        cout << "User registered successfully!" << endl;
    }

    // Method to update user password in the database
    void updatePassword(string new_password) {
        if (connection == nullptr) {
            connectToDatabase();
        }
        string query = "UPDATE users SET password='" + new_password + "' WHERE id=" + to_string(id) + "";
        if (mysql_query(connection, query.c_str()) != 0) {
            cerr << "Error updating password: " << mysql_error(connection) << endl;
            exit(1);
        }
        cout << "Password updated successfully!" << endl;
    }

    // Method to delete user from the database
    void deleteUser() {
        if (connection == nullptr) {
            connectToDatabase();
        }
        string query = "DELETE FROM users WHERE id=" + to_string(id) + "";
        if (mysql_query(connection, query.c_str()) != 0) {
            cerr << "Error deleting user: " << mysql_error(connection) << endl;
            exit(1);
        }
        cout << "User deleted successfully!" << endl;
    }
};

class Student : public User {
private:
    vector<int> enrolledCourseIDs;
    vector<float> courseResults;

public:
    Student(int _id, string _username, string _password, string _role)
        : User(_id, _username, _password, _role) {}

    void enrollCourse(int courseID) {
        enrolledCourseIDs.push_back(courseID);
        courseResults.push_back(0.0f); // Initialize result to 0.0
        if (connection == nullptr) {
            connectToDatabase();
        }
        string query = "INSERT INTO student_courses (student_id, course_id) VALUES (" + to_string(id) + ", " + to_string(courseID) + ")";
        if (mysql_query(connection, query.c_str()) != 0) {
            cerr << "Error enrolling student in course: " << mysql_error(connection) << endl;
            exit(1);
        }
    }

    void unenrollCourse(int courseID) {
        auto it = find(enrolledCourseIDs.begin(), enrolledCourseIDs.end(), courseID);
        if (it != enrolledCourseIDs.end()) {
            enrolledCourseIDs.erase(it);
            courseResults.erase(courseResults.begin() + (it - enrolledCourseIDs.begin()));
            if (connection == nullptr) {
                connectToDatabase();
            }
            string query = "DELETE FROM student_courses WHERE student_id=" + to_string(id) + " AND course_id=" + to_string(courseID);
            if (mysql_query(connection, query.c_str()) != 0) {
                cerr << "Error unenrolling student from course: " << mysql_error(connection) << endl;
                exit(1);
            }
        }
    }

    vector<int> getEnrolledCourses() {
        return enrolledCourseIDs;
    }

    vector<float> getCourseResults() {
        return courseResults;
    }

    void viewTimeTable(const string& filePath) {
        // Implement logic to view timetable in PDF form
    }
};

class Course {
private:
    int id;
    string name;
    string description;
    int instructorID;

public:
    Course(int _id, const string& _name, const string& _description, int _instructorID)
        : id(_id), name(_name), description(_description), instructorID(_instructorID) {}

    int getID() const {
        return id;
    }

    string getName() const {
        return name;
    }

    string getDescription() const {
        return description;
    }

    int getInstructorID() const {
        return instructorID;
    }
};

class Faculty : public User {
public:
    Faculty(int _id, string _username, string _password, string _role)
        : User(_id, _username, _password, _role) {}

    void uploadTimeTable(const string& filePath) {
        // Implement logic to upload timetable in PDF form
    }

    void uploadResults(int courseID, int studentID, float result) {
        if (connection == nullptr) {
            connectToDatabase();
        }
        string query = "UPDATE student_courses SET result=" + to_string(result) + " WHERE student_id=" + to_string(studentID) + " AND course_id=" + to_string(courseID);
        if (mysql_query(connection, query.c_str()) != 0) {
            cerr << "Error updating student results: " << mysql_error(connection) << endl;
            exit(1);
        }
    }
};
//create  tables if does not exist

void createTables(MYSQL* connection) {
    string createUsersTableQuery = "CREATE TABLE IF NOT EXISTS users ("
                                  "id INT AUTO_INCREMENT PRIMARY KEY,"
                                  "username VARCHAR(255) NOT NULL UNIQUE,"
                                  "password VARCHAR(255) NOT NULL,"
                                  "role VARCHAR(50) NOT NULL"
                                  ")";

    string createCoursesTableQuery = "CREATE TABLE IF NOT EXISTS courses ("
                                    "id INT AUTO_INCREMENT PRIMARY KEY,"
                                    "name VARCHAR(255) NOT NULL,"
                                    "description TEXT,"
                                    "instructor_id INT NOT NULL,"
                                    "FOREIGN KEY (instructor_id) REFERENCES users(id)"
                                    ")";

    string createStudentCoursesTableQuery = "CREATE TABLE IF NOT EXISTS student_courses ("
                                            "id INT AUTO_INCREMENT PRIMARY KEY,"
                                            "student_id INT NOT NULL,"
                                            "course_id INT NOT NULL,"
                                            "result FLOAT DEFAULT 0.0,"
                                            "FOREIGN KEY (student_id) REFERENCES users(id),"
                                            "FOREIGN KEY (course_id) REFERENCES courses(id)"
                                            ")";

    if (mysql_query(connection, createUsersTableQuery.c_str()) != 0) {
        cerr << "Error creating users table: " << mysql_error(connection) << endl;
        exit(1);
    }

    if (mysql_query(connection, createCoursesTableQuery.c_str()) != 0) {
        cerr << "Error creating courses table: " << mysql_error(connection) << endl;
        exit(1);
    }

    if (mysql_query(connection, createStudentCoursesTableQuery.c_str()) != 0) {
        cerr << "Error creating student_courses table: " << mysql_error(connection) << endl;
        exit(1);
    }
}
int main() {
    MYSQL* connection = mysql_init(NULL);
    if (connection == nullptr) {
        cerr << "Error initializing MySQL: " << mysql_error(connection) << endl;
        exit(1);
    }
    if (!mysql_real_connect(connection, "localhost", "root", "thesun", "Falcon", 0, NULL, 0)) {
        cerr << "Error connecting to MySQL: " << mysql_error(connection) << endl;
        exit(1);
    }

    createTables(connection);

    string choice;
    while (true) {
        cout << "Choose an option:" << endl;
        cout << "1. Login" << endl;
        cout << "2. Register" << endl;
        cout << "3. Enroll in Course" << endl;
        cout << "4. Unenroll from Course" << endl;
        cout << "5. View Enrolled Courses" << endl;
        cout << "6. View Course Results" << endl;
        cout << "7. Upload Timetable (Faculty)" << endl;
        cout << "8. Upload Results (Faculty)" << endl;
        cout << "9. Update Password" << endl;
        cout << "10. Delete User" << endl;
        cout << "11. Exit" << endl;
        cout << "Enter your choice: ";
        cin >> choice;

        if (choice == "1") {
            // Code for option 1
        } else if (choice == "2") {
            // Code for option 2
        } else if (choice == "3") {
            // Code for option 3
        } else if (choice == "4") {
            string username;
            cout << "Enter your username: ";
            cin >> username;
            unique_ptr<User> authenticatedUser(new User(0, username, "", ""));
            if (authenticatedUser->authenticate()) {
                if (authenticatedUser->getRole() == "student") {
                    Student* student = dynamic_cast<Student*>(authenticatedUser.get());
                    cout << "Your enrolled courses:" << endl;
                    vector<int> enrolledCourses = student->getEnrolledCourses();
                    for (int courseID : enrolledCourses) {
                        string courseQuery = "SELECT name FROM courses WHERE id=" + to_string(courseID);
                        if (mysql_query(connection, courseQuery.c_str()) == 0) {
                            MYSQL_RES* courseResult = mysql_store_result(connection);
                            if (courseResult != nullptr) {
                                MYSQL_ROW row = mysql_fetch_row(courseResult);
                                cout << "Course ID: " << courseID << ", Course Name: " << row[0] << endl;
                                mysql_free_result(courseResult);
                            } else {
                                cerr << "Error fetching course name: " << mysql_error(connection) << endl;
                            }
                        } else {
                            cerr << "Error executing course query: " << mysql_error(connection) << endl;
                        }
                    }

                    int courseID;
                    cout << "Enter the course ID you want to unenroll from: ";
                    cin >> courseID;
                    student->unenrollCourse(courseID);
                } else {
                    cout << "Only students can view their enrolled courses." << endl;
                }
            } else {
                cout << "Invalid username!" << endl;
            }
        } else if (choice == "5") {
            string username;
            cout << "Enter your username: ";
            cin >> username;
            unique_ptr<User> authenticatedUser(new User(0, username, "", ""));
            if (authenticatedUser->authenticate()) {
                if (authenticatedUser->getRole() == "student") {
                    Student* student = dynamic_cast<Student*>(authenticatedUser.get());
                    vector<int> enrolledCourses = student->getEnrolledCourses();
                    cout << "Your enrolled courses:" << endl;
                    for (int courseID : enrolledCourses) {
                        string courseQuery = "SELECT name FROM courses WHERE id=" + to_string(courseID);
                        if (mysql_query(connection, courseQuery.c_str()) == 0) {
                            MYSQL_RES* courseResult = mysql_store_result(connection);
                            if (courseResult != nullptr) {
                                MYSQL_ROW row = mysql_fetch_row(courseResult);
                                cout << "Course ID: " << courseID << ", Course Name: " << row[0] << endl;
                                mysql_free_result(courseResult);
                            } else {
                                cerr << "Error fetching course name: " << mysql_error(connection) << endl;
                            }
                        } else {
                            cerr << "Error executing course query: " << mysql_error(connection) << endl;
                        }
                    }
                } else {
                    cout << "Only students can view their enrolled courses." << endl;
                }
            } else {
                cout << "Invalid username!" << endl;
            }
        } else if (choice == "6") {
            string username;
            cout << "Enter your username: ";
            cin >> username;
            unique_ptr<User> authenticatedUser(new User(0, username, "", ""));
            if (authenticatedUser->authenticate()) {
                if (authenticatedUser->getRole() == "student") {
                    Student* student = dynamic_cast<Student*>(authenticatedUser.get());
                    vector<int> enrolledCourses = student->getEnrolledCourses();
                    vector<float> courseResults = student->getCourseResults();
                    cout << "Your course results:" << endl;
                    for (size_t i = 0; i < enrolledCourses.size(); i++) {
                        string courseQuery = "SELECT name FROM courses WHERE id=" + to_string(enrolledCourses[i]);
                        if (mysql_query(connection, courseQuery.c_str()) == 0) {
                            MYSQL_RES* courseResult = mysql_store_result(connection);
                            if (courseResult != nullptr) {
                                MYSQL_ROW row = mysql_fetch_row(courseResult);
                                cout << "Course ID: " << enrolledCourses[i] << ", Course Name: " << row[0] << ", Result: " << courseResults[i] << endl;
                                mysql_free_result(courseResult);
                            } else {
                                cerr << "Error fetching course name: " << mysql_error(connection) << endl;
                            }
                        } else {
                            cerr << "Error executing course query: " << mysql_error(connection) << endl;
                        }
                    }
                } else {
                    cout << "Only students can view their course results." << endl;
                }
            } else {
                cout << "Invalid username!" << endl;
            }
        } else if (choice == "7") {
            string username;
            cout << "Enter your username: ";
            cin >> username;
            unique_ptr<User> authenticatedUser(new User(0, username, "", ""));
            if (authenticatedUser->authenticate()) {
                if (authenticatedUser->getRole() == "faculty") {
                    Faculty* faculty = dynamic_cast<Faculty*>(authenticatedUser.get());
                    string filePath;
                    cout << "Enter the file path for the timetable: ";
                    cin >> filePath;
                    faculty->uploadTimeTable(filePath);
                } else {
                    cout << "Only faculty members can upload the timetable." << endl;
                }
            } else {
                cout << "Invalid username!" << endl;
            }
        } else if (choice == "8") {
            string username;
            cout << "Enter your username: ";
            cin >> username;
        } else if (choice == "8") {
            string username;
            cout << "Enter your username: ";
            cin >> username;
            unique_ptr<User> authenticatedUser(new User(0, username, "", ""));
            if (authenticatedUser->authenticate()) {
                if (authenticatedUser->getRole() == "faculty") {
                    Faculty* faculty = dynamic_cast<Faculty*>(authenticatedUser.get());
                    int courseID, studentID;
                    float result;
                    cout << "Enter the course ID: ";
                    cin >> courseID;
                    cout << "Enter the student ID: ";
                    cin >> studentID;
                    cout << "Enter the result: ";
                    cin >> result;
                    faculty->uploadResults(courseID, studentID, result);
                } else {
                    cout << "Only faculty members can upload results." << endl;
                }
            } else {
                cout << "Invalid username!" << endl;
            }
        } else if (choice == "9") {
            string username, newPassword;
            cout << "Enter your username: ";
            cin >> username;
            cout << "Enter your new password: ";
            cin >> newPassword;
            unique_ptr<User> authenticatedUser(new User(0, username, "", ""));
            if (authenticatedUser->authenticate()) {
                authenticatedUser->updatePassword(newPassword);
            } else {
                cout << "Invalid username!" << endl;
            }
        } else if (choice == "10") {
            string username;
            cout << "Enter your username: ";
            cin >> username;
            unique_ptr<User> authenticatedUser(new User(0, username, "", ""));
            if (authenticatedUser->authenticate()) {
                authenticatedUser->deleteUser();
            } else {
                cout << "Invalid username!" << endl;
            }
        } else if (choice == "11") {
            break;
        } else {
            cout << "Invalid choice! Please enter a valid option." << endl;
        }
    }

    mysql_close(connection);
    return 0;
}
