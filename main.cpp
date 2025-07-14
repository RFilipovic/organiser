#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <algorithm>
#include <iomanip>
#include <ctime>
#include <sstream>
#include <limits>

struct Time {
    int hour, minute;
    bool operator<(const Time& other) const {
        return hour < other.hour || (hour == other.hour && minute < other.minute);
    }
    bool operator<=(const Time& other) const {
        return *this < other || (hour == other.hour && minute == other.minute);
    }
    bool operator>=(const Time& other) const {
        return !(*this < other);
    }
    std::string toString() const {
        std::ostringstream oss;
        oss << std::setw(2) << std::setfill('0') << hour
            << ":" << std::setw(2) << std::setfill('0') << minute;
        return oss.str();
    }
};

struct Activity {
    std::string name;
    Time start, end;
};

std::map<std::string, std::vector<Activity>> schedule;

bool isConflict(const std::vector<Activity>& activities, const Time& start, const Time& end) {
    for (const auto& act : activities) {
        if (!(end <= act.start || start >= act.end)) {
            return true;
        }
    }
    return false;
}

void addActivity() {
    std::string day, name;
    int sh, sm, eh, em;

    std::cout << "Unesite dan (pon, uto, sri, cet, pet, sub, ned): ";
    std::cin >> day;
    std::cout << "Unesite naziv aktivnosti: ";
    std::cin.ignore();
    std::getline(std::cin, name);
    std::cout << "Unesite vreme pocetka (hh mm): ";
    std::cin >> sh >> sm;
    std::cout << "Unesite vreme kraja (hh mm): ";
    std::cin >> eh >> em;

    Time start = {sh, sm};
    Time end = {eh, em};

    if (end <= start) {
        std::cout << "Greska: Vreme kraja mora biti posle vremena pocetka.\n";
        return;
    }

    auto& activities = schedule[day];
    if (isConflict(activities, start, end)) {
        std::cout << "Greska: Aktivnost se preklapa sa postojecim terminom.\n";
        return;
    }

    activities.push_back({name, start, end});
    std::sort(activities.begin(), activities.end(), [](const Activity& a, const Activity& b) {
        return a.start < b.start;
    });

    std::cout << "Aktivnost dodata uspesno.\n";
}

void saveToFile(const std::string& filename) {
    std::ofstream file(filename);
    std::time_t now = std::time(nullptr);
    file << "Schedule created on: " << std::put_time(std::localtime(&now), "%Y-%m-%d") << "\n\n";

    for (const auto& [day, activities] : schedule) {
        file << day << "\n";
        for (const auto& act : activities) {
            file << "\t" << act.name << " / "
                 << act.start.toString() << "-" << act.end.toString() << "\n";
        }
        file << "\n";
    }

    file.close();
    std::cout << "Sacuvano u fajl: " << filename << "\n";
}

void displayMenu() {
    std::cout << "\nIzaberite opciju:\n";
    std::cout << "1. Dodaj aktivnost\n";
    std::cout << "2. Sacuvaj raspored\n";
    std::cout << "3. Izlaz\n";
    std::cout << "> ";
}

int main() {
    int choice;

    while (true) {
        displayMenu();
        std::cin >> choice;

        if (std::cin.fail()) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "Unesite broj izmedju 1 i 3.\n";
            continue;
        }

        switch (choice) {
            case 1:
                addActivity();
                break;
            case 2:
                saveToFile("schedule.txt");
                break;
            case 3:
                return 0;
            default:
                std::cout << "Nevalidan izbor. Unesite broj izmedju 1 i 3.\n";
                break;
        }
    }

    return 0;
}