#include <iostream>
#include <vector>
#include <string>
#include <cstdlib>
#include <fstream>
#include <cctype>
#include <algorithm>
#include <sstream>
#include <set>
#include <iomanip>

using namespace std;

enum CHOISE {
    SHOW_NOTES = 1,
    ADD_NOTE = 2,
    FIND_NOTE = 3,
    DELETE_NOTE = 4,
    DELETE_ALL_NOTES = 5,
    EDIT_NOTE = 6,
    SHOW_OF_CATEGORY,
    EXIT
};

struct Note {
    string text;
    string deadline;
    string category;
};

void to_lower(string& str) {
    for (size_t i = 0; i < str.size(); ++i) {
        unsigned char c = str[i];
        str[i] = tolower(c);
    }
}

static string trim(const string& s) {
    size_t a = s.find_first_not_of(" \t\r\n");
    size_t b = s.find_last_not_of(" \t\r\n");
    if (a == string::npos) return "";
    return s.substr(a, b - a + 1);
}

static vector<string> split_by_pipe(const string& line) {
    vector<string> parts;
    string cur;
    for (char c : line) {
        if (c == '|') {
            parts.push_back(trim(cur));
            cur.clear();
        }
        else {
            cur.push_back(c);
        }
    }
    parts.push_back(trim(cur));
    return parts;
}

static bool looks_like_deadline(const string& raw) {
    istringstream iss(raw);
    string y, mo, d, h, m;
    if (!(iss >> y >> mo >> d >> h >> m)) return false;
    auto is_num = [](const string& s) {
        if (s.empty()) return false;
        for (char c : s) if (!isdigit((unsigned char)c)) return false;
        return true;
        };
    return is_num(y) && is_num(mo) && is_num(d) && is_num(h) && is_num(m);
}

static string compose_note_line(const string& text, const string& deadline, const string& category) {
    if (!deadline.empty() && !category.empty())
        return text + " | " + deadline + " | " + category;
    if (!deadline.empty() && category.empty())
        return text + " | " + deadline;
    if (deadline.empty() && !category.empty())
        return text + " |  | " + category;
    return text;
}

void rewrite_notes(vector<string>& notes) {
    ofstream file("notex.txt", ios::trunc);
    for (auto& note : notes) {
        file << note << '\n';
    }
    file.close();
}

void show_menu() {
    cout << "[1]Show all notes" << endl;
    cout << "[2]Add note" << endl;
    cout << "[3]Find note" << endl;
    cout << "[4]Delete note" << endl;
    cout << "[5]Delete all notes" << endl;
    cout << "[6]Edit note" << endl;
    cout << "[7]Show notes of category" << endl;
    cout << "[8]Exit" << endl;
}

void show_notes() {
    ifstream file("notex.txt");

    if (!file.is_open()) {
        cout << "No notes exist" << endl;
        return;
    }
    string showing_note;
    for (int i = 1; !file.eof(); i++) {
        getline(file, showing_note);
        if (!showing_note.empty()) {
            cout << i << " - " << showing_note << endl;
        }
    }
    file.close();
}

string deadline_input() {
    cout << "Deadline? (y/n): ";
    char r;
    cin >> r; cin.ignore();
    if (r != 'y' && r != 'Y') return "";

    cout << "Enter deadline in format: YYYY MM DD HH MM\nDeadline: ";
    string deadline_line;
    getline(cin, deadline_line);
    deadline_line = trim(deadline_line);
    return deadline_line;
}

static string extract_category_from_line(const string& line) {
    auto parts = split_by_pipe(line);
    if (parts.size() >= 3) return parts[2];
    if (parts.size() == 2) {
        return looks_like_deadline(parts[1]) ? string() : parts[1];
    }
    return "";
}

vector<string> list_cat_unique() {
    vector<string> lines;
    ifstream file("notex.txt");
    string current_line;
    while (getline(file, current_line)) {
        current_line = trim(current_line);
        if (!current_line.empty()) lines.push_back(current_line);
    }
    set<string> uniq;
    for (auto& ln : lines) {
        string cat = extract_category_from_line(ln);
        if (!cat.empty()) uniq.insert(cat);
    }
    return vector<string>(uniq.begin(), uniq.end());
}

void show_categories() {
    vector<string> cats = list_cat_unique();
    if (cats.empty()) { cout << "No categories" << endl; return; }
    for (int i = 0; i < (int)cats.size(); ++i) {
        cout << (i + 1) << " - " << cats[i] << endl;
    }
}

string category_input() {
    cout << "Category?(y/n): ";
    char r;
    cin >> r; cin.ignore();
    if (r != 'y' && r != 'Y') return "";

    cout << "Created categories:" << endl;
    show_categories();
    cout << "Category name (existing or new): ";
    string cat_line;
    getline(cin, cat_line);
    cat_line = trim(cat_line);
    cat_line.erase(remove(cat_line.begin(), cat_line.end(), '|'), cat_line.end());
    return cat_line;
}

void add_note(string& new_note) {
    string dl = deadline_input();
    string cat = category_input();
    ofstream file("notex.txt", ios::app);
    if (!file.is_open()) {
        cout << "File is not opened" << endl;
        exit(EXIT_FAILURE);
    }
    string line = compose_note_line(new_note, dl, cat);
    file << line << endl;

    file.close();

    cout << "Done" << endl;
}

vector<string> find_note(string& keyword) {
    vector<string> result;
    ifstream file("notex.txt");
    if (!file.is_open()) {
        cout << "Empty notes" << endl;
        return result;
    }
    string kw = keyword;
    to_lower(kw);

    string current_line;
    while (getline(file, current_line)) {
        if (current_line.empty()) continue;
        string lw_line = current_line;
        to_lower(lw_line);
        if (lw_line.find(kw) != string::npos) {
            result.push_back(current_line);
        }
    }
    file.close();
    return result;
}

void delete_note(int number) {
    if (number <= 0) {
        cout << "Invalid number" << endl;
        return;
    }
    vector<string> notes;
    ifstream file("notex.txt");
    string current_line;
    while (getline(file, current_line)) {
        if (!current_line.empty()) notes.push_back(current_line);
    }
    if (number > (int)notes.size()) {
        cout << "Invalid number" << endl;
        return;
    }
    notes.erase(notes.begin() + number - 1);
    rewrite_notes(notes);
    cout << "Done" << endl;
    file.close();
}

void delete_all_notes() {
    ofstream file("notex.txt", ios::trunc);
    cout << "Done" << endl;
    file.close();
}

void edit_note(int num, string& new_note) {
    if (num <= 0) {
        cout << "Invalid number" << endl;
        return;
    }
    vector<string> notes;
    ifstream file("notex.txt");
    string current_line;
    while (getline(file, current_line)) {
        if (!current_line.empty()) notes.push_back(current_line);
    }
    if (num > (int)notes.size()) {
        cout << "Invalid number" << endl;
        return;
    }

    string dl = deadline_input();
    string cat = category_input();

    notes[num - 1] = compose_note_line(new_note, dl, cat);
    rewrite_notes(notes);
    file.close();
    cout << "Done" << endl;
}

static void show_notes_of_category() {
    system("cls");
    cout << "Categories:" << endl;
    show_categories();
    cout << "Enter category to filter: ";
    string cat;
    getline(cin, cat);
    cat = trim(cat);

    ifstream file("notex.txt");
    if (!file.is_open()) {
        cout << "No notes exist" << endl;
        return;
    }
    string line;
    int idx = 1;
    bool any = false;
    while (getline(file, line)) {
        string c = extract_category_from_line(line);
        if (c == cat && !trim(line).empty()) {
            cout << idx << " - " << line << endl;
            any = true;
        }
        if (!trim(line).empty()) idx++;
    }
    if (!any) cout << "No notes in this category" << endl;
    file.close();
}

int main()
{
    while (true) {
        show_menu();
        int option = 0;

        cout << "Your option: ";
        cin >> option;

        if (option == SHOW_NOTES) {
            system("cls");
            show_notes();
            system("pause");
        }
        else if (option == ADD_NOTE) {
            system("cls");
            string new_note;
            getline(cin, new_note);
            cout << "Note: ";
            getline(cin, new_note);
            add_note(new_note);
            system("pause");
        }
        else if (option == FIND_NOTE) {
            system("cls");
            string note_to_find;
            getline(cin, note_to_find);
            cout << "Enter note to find: ";
            getline(cin, note_to_find);
            vector<string> result = find_note(note_to_find);
            if (result.empty()) {
                cout << "No notes matches" << endl;
            }
            else {
                cout << "All found notes with matches: " << endl;
                for (size_t i = 0; i < result.size(); ++i) {
                    cout << (i + 1) << " - " << result[i] << endl;
                }
            }
            system("pause");
        }
        else if (option == DELETE_NOTE) {
            system("cls");
            int number;
            show_notes();
            cout << "Enter note number to delete: ";
            cin >> number;
            delete_note(number);
            system("pause");
        }
        else if (option == DELETE_ALL_NOTES) {
            system("cls");
            delete_all_notes();
            system("pause");
        }
        else if (option == EDIT_NOTE) {
            system("cls");
            int num;
            show_notes();
            cout << "Enter note number to edit: ";
            cin >> num;
            cin.ignore();
            cout << "Enter new note: ";
            string new_note;
            getline(cin, new_note);
            edit_note(num, new_note);
            system("pause");
        }
        else if (option == SHOW_OF_CATEGORY) {
            system("cls");
            cin.ignore();
            show_notes_of_category();
            system("pause");
        }
        else if (option == EXIT) {
            system("cls");
            break;
        }
        else
            break;
    }

    return 0;
}
