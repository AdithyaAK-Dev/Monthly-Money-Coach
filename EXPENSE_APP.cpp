//Including all neccesary header files as per problem requirements
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <map>
#include <iomanip>
#include <stdexcept>
#include <algorithm>

using namespace std;


//defining expense class

class Expense {
private:
    //date,category,amount and notes as members
    string trans_date;
    string category_name;
    double spent_amount;
    string notes;

public:
    //expense constructor when object created
    Expense() : spent_amount(0.0) {}

    //constructor to set data without being modified (using const)
    Expense(const string& d, const string& c, double a, const string& n) {
        trans_date = d;
        category_name = c;
        spent_amount = a;
        notes = n;
    }


    //our getters that safely return data when requested down the line
    string get_date() const { 
        return trans_date; 
    }
    
    int get_month() const {
        return stoi(trans_date.substr(3, 2));
    }

    string get_category() const { 
        return category_name; 
    }
    
    double get_amount() const { 
        return spent_amount; 
    }

    //displaying expense data in neat,formatted manner
    void display_row() const {
        cout << left << setw(12) << trans_date
             << setw(18) << category_name
             << setw(12) << fixed << setprecision(2) << spent_amount
             << notes << "\n";
    }

    //converting data to csv format, to be stored into csv file later on
    string to_csv() const {
        return trans_date + "," + category_name + "," + to_string(spent_amount) + "," + notes;
    }
};


//Budget manager class definition
class BudgetManager {
private:
    //expense list, category vectors , 
    vector<Expense> expense_list;
    vector<string> categories;
    
    //storing our default budget and any custom monthly limits
    double base_limit;
    map<int, double> month_limits;
    
    //file names used for persistent storage
    string data_file;
    string config_file;
    string cat_file;

    //helper function to sort our categories in descending order based on amount spent
    static bool sort_descending(const pair<string, double>& a, const pair<string, double>& b) {
        return a.second > b.second;
    }

    //returning the correct month name string based on integer input
    string get_month_name(int m) const {
        const char* months[] = {"", "Jan", "Feb", "Mar", "Apr", "May", "Jun", 
                                "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
        if (m >= 1 && m <= 12) return months[m];
        return "Unknown";
    }

    //getting a valid decimal number from user and handling invalid text inputs safely
    double get_valid_double(const string& prompt_text) const {
        string input_str;
        double val;
        while (true) {
            cout << prompt_text;
            getline(cin, input_str);
            try {
                size_t pos;
                val = stod(input_str, &pos);
                if (pos == input_str.length()) {
                    return val;
                }
            } catch (...) {
            }
            cout << "Invalid input. Please enter a number.\n";
        }
    }

    //getting a valid whole number from user without crashing the program
    int get_valid_int(const string& prompt_text) const {
        string input_str;
        int val;
        while (true) {
            cout << prompt_text;
            getline(cin, input_str);
            try {
                size_t pos;
                val = stoi(input_str, &pos);
                if (pos == input_str.length()) {
                    return val;
                }
            } catch (...) {
            }
            cout << "Invalid input. Please enter an integer.\n";
        }
    }

    //checking if the date entered is actually possible and follows DD-MM format
    bool check_date_format(const string& date_str) const {
        if (date_str.length() != 5 || date_str[2] != '-') {
            return false;
        }
        try {
            int d = stoi(date_str.substr(0, 2));
            int m = stoi(date_str.substr(3, 2));
            if (m < 1 || m > 12) return false;
            int days_in_month[] = {0, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
            if (d < 1 || d > days_in_month[m]) return false;
            return true;
        } catch (...) {
            return false;
        }
    }

    //reading our budget limits from the text file on startup
    void load_settings() {
        ifstream config_in(config_file);
        if (config_in.is_open()) {
            string line;
            if (getline(config_in, line) && !line.empty()) {
                try {
                    base_limit = stod(line);
                } catch (...) {
                    base_limit = 0.0;
                }
            }
            while (getline(config_in, line)) {
                if (line.empty()) continue;
                size_t pos = line.find(',');
                if (pos != string::npos) {
                    try {
                        int m = stoi(line.substr(0, pos));
                        double val = stod(line.substr(pos + 1));
                        month_limits[m] = val;
                    } catch (...) {}
                }
            }
            config_in.close();
        } else {
            //setting up for the first time if no config file is found
            base_limit = get_valid_double("First time setup -> Enter your default monthly budget: Rs. ");
            save_settings();
        }
    }

    //writing our current budget limits back to the text file
    void save_settings() const {
        ofstream config_out(config_file);
        if (config_out.is_open()) {
            config_out << base_limit << "\n";
            for (map<int, double>::const_iterator it = month_limits.begin(); it != month_limits.end(); ++it) {
                config_out << it->first << "," << fixed << setprecision(2) << it->second << "\n";
            }
            config_out.close();
        }
    }

    //checking if a custom limit exists for the month, otherwise returning default limit
    double get_current_limit(int m) const {
        map<int, double>::const_iterator it = month_limits.find(m);
        if (it != month_limits.end()) {
            return it->second;
        }
        return base_limit;
    }

    //loading custom categories from file, or setting defaults if its the first time
    void load_categories() {
        ifstream cat_in(cat_file);
        string line;
        bool loaded = false;
        if (cat_in.is_open()) {
            while (getline(cat_in, line)) {
                if (!line.empty()) {
                    categories.push_back(line);
                    loaded = true;
                }
            }
            cat_in.close();
        }
        if (!loaded) {
            categories.push_back("Food");
            categories.push_back("Transport");
            categories.push_back("Utilities");
            categories.push_back("Fun");
            categories.push_back("Health");
            categories.push_back("Other");
            save_categories();
        }
    }

    //saving our updated category list to the text file
    void save_categories() const {
        ofstream cat_out(cat_file);
        if (cat_out.is_open()) {
            for (size_t i = 0; i < categories.size(); ++i) {
                cat_out << categories[i] << "\n";
            }
            cat_out.close();
        }
    }

    //reading all our past expenses from the csv file when program starts
    void load_data() {
        ifstream data_in(data_file);
        string line;
        if (data_in.is_open()) {
            while (getline(data_in, line)) {
                if (line.empty()) continue;
                size_t p1 = line.find(',');
                size_t p2 = line.find(',', p1 + 1);
                size_t p3 = line.find(',', p2 + 1);
                if (p1 != string::npos && p2 != string::npos && p3 != string::npos) {
                    string d = line.substr(0, p1);
                    string c = line.substr(p1 + 1, p2 - p1 - 1);
                    try {
                        double a = stod(line.substr(p2 + 1, p3 - p2 - 1));
                        string r = line.substr(p3 + 1);
                        expense_list.push_back(Expense(d, c, a, r));
                    } catch (...) {
                    }
                }
            }
            data_in.close();
        }
    }

    //converting strings to lowercase to easily check for duplicate categories
    string to_lower(const string& str) const {
        string res = str;
        for (size_t i = 0; i < res.length(); ++i) {
            if (res[i] >= 'A' && res[i] <= 'Z') {
                res[i] = res[i] + ('a' - 'A');
            }
        }
        return res;
    }

public:
    //constructor that sets file names and loads all our saved data automatically
    BudgetManager() {
        data_file = "data.csv";
        config_file = "budget.txt";
        cat_file = "categories.txt";
        load_settings();
        load_categories();
        load_data();
    }

    //writing our entire expense list back to the csv file
    void save_data() const {
        ofstream data_out(data_file);
        if (data_out.is_open()) {
            for (size_t i = 0; i < expense_list.size(); ++i) {
                data_out << expense_list[i].to_csv() << "\n";
            }
            data_out.close();
        }
    }

    //menu to let user add categories or change their budget limits
    void run_settings() {
        while (true) {
            cout << "\n--- App Settings ---\n";
            cout << "1. Add a new category\n";
            cout << "2. Change default budget\n";
            cout << "3. Set custom budget for a specific month\n";
            cout << "4. Go back\n";
            
            int choice = get_valid_int("Choice: ");
            
            if (choice == 1) {
                cout << "\nCurrent Categories:\n";
                for (size_t i = 0; i < categories.size(); ++i) {
                    cout << i + 1 << ". " << categories[i] << "\n";
                }
                string new_cat;
                cout << "Type new category name (or 'cancel'): ";
                getline(cin, new_cat);
                if (to_lower(new_cat) != "cancel" && !new_cat.empty()) {
                    bool exists = false;
                    for (size_t i = 0; i < categories.size(); ++i) {
                        if (to_lower(categories[i]) == to_lower(new_cat)) {
                            exists = true;
                            break;
                        }
                    }
                    if (exists) {
                        cout << "Category already exists.\n";
                    } else {
                        categories.push_back(new_cat);
                        save_categories();
                        cout << "Category added!\n";
                    }
                }
            } else if (choice == 2) {
                cout << "\nCurrent default budget: Rs. " << fixed << setprecision(2) << base_limit << "\n";
                base_limit = get_valid_double("Enter new default budget: Rs. ");
                save_settings();
                cout << "Default budget updated.\n";
            } else if (choice == 3) {
                int m;
                while (true) {
                    m = get_valid_int("Enter month number (1-12): ");
                    if (m >= 1 && m <= 12) break;
                    cout << "Invalid month.\n";
                }
                cout << "Current budget for " << get_month_name(m) << ": Rs. " << fixed << setprecision(2) << get_current_limit(m) << "\n";
                double new_limit = get_valid_double("Enter new custom budget: Rs. ");
                month_limits[m] = new_limit;
                save_settings();
                cout << "Custom budget set for " << get_month_name(m) << ".\n";
            } else if (choice == 4) {
                break;
            } else {
                cout << "Invalid choice.\n";
            }
        }
    }

    //getting input from user to log a new expense and saving it immediately
    void add_new_expense() {
        string d, c, n;
        double a;

        while (true) {
            cout << "Enter Date (DD-MM): ";
            getline(cin, d);
            if (check_date_format(d)) break;
            cout << "Invalid date. Make sure it exists and uses DD-MM format.\n";
        }

        cout << "\nCategories:\n";
        for (size_t i = 0; i < categories.size(); ++i) {
            cout << i + 1 << ". " << categories[i] << "\n";
        }
        
        int cat_num;
        while (true) {
            cat_num = get_valid_int("Pick a category number: ");
            if (cat_num >= 1 && cat_num <= static_cast<int>(categories.size())) {
                c = categories[cat_num - 1];
                break;
            }
            cout << "Invalid selection.\n";
        }

        while (true) {
            a = get_valid_double("Amount spent: Rs. ");
            if (a > 0) break;
            cout << "Amount must be greater than zero.\n";
        }

        cout << "Add a note: ";
        getline(cin, n);

        expense_list.push_back(Expense(d, c, a, n));
        save_data();
        cout << "Expense saved!\n";
    }

    //displaying all our logged expenses in a neat table format
    void show_all() const {
        if (expense_list.empty()) {
            cout << "\nNo expenses logged yet.\n";
            return;
        }
        cout << "\n" << left << setw(12) << "DATE" << setw(18) << "CATEGORY" << setw(12) << "AMOUNT" << "NOTE\n";
        cout << string(60, '-') << "\n";
        for (size_t i = 0; i < expense_list.size(); ++i) {
            expense_list[i].display_row();
        }
    }

    //letting user search their past expenses by specific date or category
    void search_data() const {
        if (expense_list.empty()) {
            cout << "\nNo data to search.\n";
            return;
        }
        int opt;
        while (true) {
            cout << "\n1. Search by Date\n2. Search by Category\n";
            opt = get_valid_int("Choice: ");
            if (opt == 1 || opt == 2) break;
        }

        string q;
        if (opt == 1) {
            while (true) {
                cout << "Enter Date (DD-MM): ";
                getline(cin, q);
                if (check_date_format(q)) break;
            }
        } else {
            cout << "\nSelect Category to Search:\n";
            for (size_t i = 0; i < categories.size(); ++i) {
                cout << i + 1 << ". " << categories[i] << "\n";
            }
            int cat_num;
            while (true) {
                cat_num = get_valid_int("Choice: ");
                if (cat_num >= 1 && cat_num <= static_cast<int>(categories.size())) {
                    q = categories[cat_num - 1];
                    break;
                }
            }
        }

        bool found = false;
        cout << "\n" << left << setw(12) << "DATE" << setw(18) << "CATEGORY" << setw(12) << "AMOUNT" << "NOTE\n";
        cout << string(60, '-') << "\n";
        for (size_t i = 0; i < expense_list.size(); ++i) {
            if ((opt == 1 && expense_list[i].get_date() == q) || 
                (opt == 2 && expense_list[i].get_category() == q)) {
                expense_list[i].display_row();
                found = true;
            }
        }
        if (!found) cout << "No matches found.\n";
    }

    //calculating and displaying a detailed breakdown of spending for a specific month
    void show_monthly_dashboard() const {
        if (expense_list.empty()) {
            cout << "\nNot enough data for a dashboard.\n";
            return;
        }
        
        int t_month;
        while (true) {
            t_month = get_valid_int("Enter month to view (1-12): ");
            if (t_month >= 1 && t_month <= 12) break;
            cout << "Invalid month.\n";
        }

        double total_spent = 0;
        map<string, double> cat_totals;
        for (size_t i = 0; i < expense_list.size(); ++i) {
            if (expense_list[i].get_month() == t_month) {
                double a = expense_list[i].get_amount();
                total_spent += a;
                cat_totals[expense_list[i].get_category()] += a;
            }
        }

        if (total_spent == 0) {
            cout << "\nNo spending logged for " << get_month_name(t_month) << ".\n";
            return;
        }

        double active_limit = get_current_limit(t_month);

        cout << "\n--- " << get_month_name(t_month) << " Dashboard ---\n";
        cout << "Budget Limit : Rs. " << fixed << setprecision(2) << active_limit;
        if (month_limits.find(t_month) != month_limits.end()) {
            cout << " (Custom)\n";
        } else {
            cout << " (Default)\n";
        }
        cout << "Amount Spent : Rs. " << total_spent << "\n";
        
        if (total_spent > active_limit) {
            cout << "Status       : OVER BUDGET by Rs. " << (total_spent - active_limit) << "\n";
        } else {
            cout << "Status       : UNDER BUDGET (Remaining: Rs. " << (active_limit - total_spent) << ")\n";
        }

        cout << "\nSpending Breakdown:\n";
        vector<pair<string, double> > sorted_cats;
        for (map<string, double>::const_iterator cit = cat_totals.begin(); cit != cat_totals.end(); ++cit) {
            sorted_cats.push_back(*cit);
        }
        sort(sorted_cats.begin(), sorted_cats.end(), sort_descending);

        for (size_t i = 0; i < sorted_cats.size(); ++i) {
            double perc = (sorted_cats[i].second / total_spent) * 100.0;
            cout << " - " << left << setw(15) << sorted_cats[i].first 
                 << " Rs. " << setw(8) << sorted_cats[i].second 
                 << " (" << fixed << setprecision(1) << perc << "%)\n";
        }
    }

    //showing a high level view of all months to see if we were over or under budget
    void show_yearly_summary() const {
        if (expense_list.empty()) {
            cout << "\nNo data available for yearly summary.\n";
            return;
        }

        map<int, double> month_totals;
        double year_total = 0;
        for (size_t i = 0; i < expense_list.size(); ++i) {
            double a = expense_list[i].get_amount();
            month_totals[expense_list[i].get_month()] += a;
            year_total += a;
        }

        cout << "\n--- Yearly Summary ---\n";
        cout << left << setw(15) << "Month" << setw(14) << "Spent" << setw(14) << "Budget" << "Status\n";
        cout << string(60, '-') << "\n";

        for (int m = 1; m <= 12; ++m) {
            if (month_totals.find(m) != month_totals.end()) {
                double spent = month_totals[m];
                double active_limit = get_current_limit(m);
                
                cout << left << setw(15) << get_month_name(m) 
                     << "Rs. " << setw(13) << fixed << setprecision(2) << spent
                     << "Rs. " << setw(13) << active_limit;
                
                if (spent > active_limit) {
                    cout << "OVER (-Rs. " << (spent - active_limit) << ")\n";
                } else {
                    cout << "SAFE (+Rs. " << (active_limit - spent) << ")\n";
                }
            }
        }
        cout << string(60, '-') << "\n";
        cout << "Total Spent This Year: Rs. " << fixed << setprecision(2) << year_total << "\n";
    }

    //displaying the main options to the user and getting their choice
    int main_menu() const {
        cout << "\n=== Monthly Money Coach ===\n";
        cout << "1. Add Expense\n";
        cout << "2. View All Expenses\n";
        cout << "3. Search Data\n";
        cout << "4. Monthly Dashboard\n";
        cout << "5. Yearly Summary\n";
        cout << "6. Settings (Categories & Limits)\n";
        cout << "7. Save & Exit\n";
        return get_valid_int("Select an option: ");
    }
};

int main() {
    //creating our main app object
    BudgetManager app;
    int choice;

    //looping the main menu until user chooses to exit
    do {
        choice = app.main_menu();
        switch (choice) {
            case 1: app.add_new_expense(); break;
            case 2: app.show_all(); break;
            case 3: app.search_data(); break;
            case 4: app.show_monthly_dashboard(); break;
            case 5: app.show_yearly_summary(); break;
            case 6: app.run_settings(); break;
            case 7: cout << "\nSaving data... Bye!\n"; break;
            default: cout << "Invalid choice. Please try again.\n";
        }
    } while (choice != 7);

    //returning 0 to indicate successful execution
    return 0;
}