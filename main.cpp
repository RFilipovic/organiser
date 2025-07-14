#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <algorithm>
#include <iomanip>
#include <ctime>
#include <sstream>
#include <limits>
#include <cctype> // Za tolower()

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
    int id; // Dodali smo ID za lakše uređivanje
};

std::map<std::string, std::vector<Activity>> schedule;
std::string currentFilename = "schedule.txt";
int nextActivityId = 1;

// Pomoćna funkcija za pretvaranje stringa u mala slova
std::string toLower(const std::string& str) {
    std::string lowerStr;
    for (char c : str) {
        lowerStr += tolower(c);
    }
    return lowerStr;
}

bool isConflict(const std::vector<Activity>& activities, const Time& start, const Time& end, int excludeId = -1) {
    for (const auto& act : activities) {
        if (act.id != excludeId && !(end <= act.start || start >= act.end)) {
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
    day = toLower(day); // Normaliziramo unos dana

    std::cout << "Unesite naziv aktivnosti: ";
    std::cin.ignore();
    std::getline(std::cin, name);

    std::cout << "Unesite vreme pocetka (hh mm): ";
    while (!(std::cin >> sh >> sm) || sh < 0 || sh > 23 || sm < 0 || sm > 59) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "Nevalidan unos. Unesite vreme ponovo (hh mm): ";
    }

    std::cout << "Unesite vreme kraja (hh mm): ";
    while (!(std::cin >> eh >> em) || eh < 0 || eh > 23 || em < 0 || em > 59) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "Nevalidan unos. Unesite vreme ponovo (hh mm): ";
    }

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

    activities.push_back({name, start, end, nextActivityId++});
    std::sort(activities.begin(), activities.end(), [](const Activity& a, const Activity& b) {
        return a.start < b.start;
    });

    std::cout << "Aktivnost dodata uspesno.\n";
}

void editActivity() {
    std::string day;
    std::cout << "Unesite dan za uređivanje (pon, uto, sri, cet, pet, sub, ned): ";
    std::cin >> day;
    day = toLower(day);

    if (schedule.find(day) == schedule.end() || schedule[day].empty()) {
        std::cout << "Nema aktivnosti za ovaj dan.\n";
        return;
    }

    std::cout << "Aktivnosti za " << day << ":\n";
    for (const auto& act : schedule[day]) {
        std::cout << act.id << ". " << act.name << " (" 
                  << act.start.toString() << " - " << act.end.toString() << ")\n";
    }

    int id;
    std::cout << "Unesite ID aktivnosti za uređivanje: ";
    std::cin >> id;

    auto it = std::find_if(schedule[day].begin(), schedule[day].end(), 
        [id](const Activity& a) { return a.id == id; });

    if (it == schedule[day].end()) {
        std::cout << "Aktivnost sa tim ID-om ne postoji.\n";
        return;
    }

    Activity& activity = *it;
    std::string newName;
    int sh, sm, eh, em;

    std::cout << "Trenutni naziv (" << activity.name << "): ";
    std::cin.ignore();
    std::getline(std::cin, newName);
    if (!newName.empty()) {
        activity.name = newName;
    }

    std::cout << "Trenutno vreme pocetka (" << activity.start.toString() << "): ";
    std::string timeInput;
    std::getline(std::cin, timeInput);
    if (!timeInput.empty()) {
        std::istringstream iss(timeInput);
        if (iss >> sh >> sm) {
            Time newStart = {sh, sm};
            if (!isConflict(schedule[day], newStart, activity.end, activity.id)) {
                activity.start = newStart;
            } else {
                std::cout << "Greska: Novo vreme pocetka stvara preklapanje.\n";
            }
        }
    }

    std::cout << "Trenutno vreme kraja (" << activity.end.toString() << "): ";
    std::getline(std::cin, timeInput);
    if (!timeInput.empty()) {
        std::istringstream iss(timeInput);
        if (iss >> eh >> em) {
            Time newEnd = {eh, em};
            if (!isConflict(schedule[day], activity.start, newEnd, activity.id)) {
                activity.end = newEnd;
            } else {
                std::cout << "Greska: Novo vreme kraja stvara preklapanje.\n";
            }
        }
    }

    std::sort(schedule[day].begin(), schedule[day].end(), [](const Activity& a, const Activity& b) {
        return a.start < b.start;
    });

    std::cout << "Aktivnost uspesno azurirana.\n";
}

void deleteActivity() {
    std::string day;
    std::cout << "Unesite dan za brisanje (pon, uto, sri, cet, pet, sub, ned): ";
    std::cin >> day;
    day = toLower(day);

    if (schedule.find(day) == schedule.end() || schedule[day].empty()) {
        std::cout << "Nema aktivnosti za ovaj dan.\n";
        return;
    }

    std::cout << "Aktivnosti za " << day << ":\n";
    for (const auto& act : schedule[day]) {
        std::cout << act.id << ". " << act.name << " (" 
                  << act.start.toString() << " - " << act.end.toString() << ")\n";
    }

    int id;
    std::cout << "Unesite ID aktivnosti za brisanje: ";
    std::cin >> id;

    auto it = std::find_if(schedule[day].begin(), schedule[day].end(), 
        [id](const Activity& a) { return a.id == id; });

    if (it == schedule[day].end()) {
        std::cout << "Aktivnost sa tim ID-om ne postoji.\n";
        return;
    }

    schedule[day].erase(it);
    std::cout << "Aktivnost uspesno obrisana.\n";
}

void saveToFile() {
    std::cout << "Trenutna datoteka: " << currentFilename << "\n";
    std::cout << "Unesite naziv datoteke (prazno za " << currentFilename << "): ";
    std::cin.ignore();
    std::string filename;
    std::getline(std::cin, filename);

    if (!filename.empty()) {
        currentFilename = filename;
    }

    std::ofstream file(currentFilename);
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
    std::cout << "Sacuvano u fajl: " << currentFilename << "\n";
}

void loadFromFile() {
    std::cout << "Trenutna datoteka: " << currentFilename << "\n";
    std::cout << "Unesite naziv datoteke za učitavanje (prazno za " << currentFilename << "): ";
    std::cin.ignore();
    std::string filename;
    std::getline(std::cin, filename);

    if (!filename.empty()) {
        currentFilename = filename;
    }

    std::ifstream file(currentFilename);
    if (!file) {
        std::cout << "Greska: Datoteka ne postoji.\n";
        return;
    }

    schedule.clear();
    nextActivityId = 1;

    std::string line, currentDay;
    while (std::getline(file, line)) {
        if (line.empty()) continue;

        // Provjeravamo da li je linija dan (nema tab karakter)
        if (line.find('\t') == std::string::npos) {
            currentDay = toLower(line);
        } else {
            // Parsiranje aktivnosti
            size_t nameEnd = line.find(" / ");
            if (nameEnd != std::string::npos) {
                Activity act;
                act.name = line.substr(1, nameEnd - 1); // Preskačemo tab
                
                size_t timeSep = line.find('-', nameEnd);
                if (timeSep != std::string::npos) {
                    std::string startTime = line.substr(nameEnd + 3, timeSep - (nameEnd + 3));
                    std::string endTime = line.substr(timeSep + 1);
                    
                    size_t colonPos = startTime.find(':');
                    if (colonPos != std::string::npos) {
                        act.start.hour = std::stoi(startTime.substr(0, colonPos));
                        act.start.minute = std::stoi(startTime.substr(colonPos + 1));
                    }
                    
                    colonPos = endTime.find(':');
                    if (colonPos != std::string::npos) {
                        act.end.hour = std::stoi(endTime.substr(0, colonPos));
                        act.end.minute = std::stoi(endTime.substr(colonPos + 1));
                    }
                    
                    act.id = nextActivityId++;
                    schedule[currentDay].push_back(act);
                }
            }
        }
    }

    file.close();
    std::cout << "Uspesno ucitano iz datoteke: " << currentFilename << "\n";
}

void printCurrentSchedule() {
    std::cout << "\nTrenutni raspored (" << currentFilename << "):\n";
    for (const auto& [day, activities] : schedule) {
        std::cout << day << "\n";
        for (const auto& act : activities) {
            std::cout << "\t" << act.name << " / "
                     << act.start.toString() << "-" << act.end.toString() << "\n";
        }
        std::cout << "\n";
    }
}

void printActivitiesForDay(const std::string& day) {
    if (schedule.find(day) == schedule.end() || schedule[day].empty()) {
        std::cout << "Nema aktivnosti za ovaj dan.\n";
        return;
    }

    std::cout << "Aktivnosti za " << day << ":\n";
    for (const auto& act : schedule[day]) {
        std::cout << act.id << ". " << act.name << " (" 
                  << act.start.toString() << " - " << act.end.toString() << ")\n";
    }
}

void copyActivity() {
    std::string sourceDay, targetDay;
    int activityId;

    std::cout << "Unesite dan iz kojeg kopirate aktivnost (pon, uto, sri, cet, pet, sub, ned): ";
    std::cin >> sourceDay;
    sourceDay = toLower(sourceDay);

    printActivitiesForDay(sourceDay);
    if (schedule.find(sourceDay) == schedule.end() || schedule[sourceDay].empty()) {
        return;
    }

    std::cout << "Unesite ID aktivnosti koju želite kopirati: ";
    std::cin >> activityId;

    auto it = std::find_if(schedule[sourceDay].begin(), schedule[sourceDay].end(), 
        [activityId](const Activity& a) { return a.id == activityId; });

    if (it == schedule[sourceDay].end()) {
        std::cout << "Aktivnost sa tim ID-om ne postoji.\n";
        return;
    }

    Activity activityToCopy = *it;

    std::cout << "Unesite dan na koji želite kopirati aktivnost (pon, uto, sri, cet, pet, sub, ned): ";
    std::cin >> targetDay;
    targetDay = toLower(targetDay);

    // Provjera preklapanja u ciljnom danu
    if (isConflict(schedule[targetDay], activityToCopy.start, activityToCopy.end)) {
        std::cout << "Greska: Aktivnost se preklapa sa postojecim terminom u ciljnom danu.\n";
        return;
    }

    // Dodajemo kopiranu aktivnost sa novim ID-om
    activityToCopy.id = nextActivityId++;
    schedule[targetDay].push_back(activityToCopy);
    std::sort(schedule[targetDay].begin(), schedule[targetDay].end(), [](const Activity& a, const Activity& b) {
        return a.start < b.start;
    });

    std::cout << "Aktivnost uspješno kopirana iz " << sourceDay << " u " << targetDay << ".\n";
}

void displayMenu() {
    std::cout << "\n===== MENI =====";
    std::cout << "\nTrenutna datoteka: " << currentFilename;
    std::cout << "\n1. Dodaj aktivnost";
    std::cout << "\n2. Uredi aktivnost";
    std::cout << "\n3. Obriši aktivnost";
    std::cout << "\n4. Kopiraj aktivnost";
    std::cout << "\n5. Prikaži raspored";
    std::cout << "\n6. Spremi raspored";
    std::cout << "\n7. Učitaj raspored";
    std::cout << "\n8. Promijeni naziv datoteke";
    std::cout << "\n9. Izlaz";
    std::cout << "\n> ";
}

int main() {
    int choice;

    while (true) {
        displayMenu();
        std::cin >> choice;

        if (std::cin.fail()) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "Unesite broj izmedju 1 i 9.\n";
            continue;
        }

        switch (choice) {
            case 1:
                addActivity();
                break;
            case 2:
                editActivity();
                break;
            case 3:
                deleteActivity();
                break;
            case 4:
                copyActivity();
                break;
            case 5:
                printCurrentSchedule();
                break;
            case 6:
                saveToFile();
                break;
            case 7:
                loadFromFile();
                break;
            case 8: {
                std::cout << "Unesite novi naziv datoteke: ";
                std::cin.ignore();
                std::getline(std::cin, currentFilename);
                std::cout << "Naziv datoteke promijenjen u: " << currentFilename << "\n";
                break;
            }
            case 9:
                return 0;
            default:
                std::cout << "Nevalidan izbor. Unesite broj izmedju 1 i 9.\n";
                break;
        }
    }

    return 0;
}