#include <bits/stdc++.h>
using namespace std;
// 1. DATA STRUCTURES
struct Movie {
    int id;
    string title;
    string director;
    int year;
    vector<double> ratings;
    double getAverageRating() const {
        if (ratings.empty()) return 0.0;
        double sum = 0;
        for (double r : ratings) sum += r;
        return sum / ratings.size();
    }
};
struct BTreeNode {
    Movie keys[3];          
    BTreeNode* children[4]; 
    int num_keys;          
    bool is_leaf;            
    BTreeNode(bool leaf) {
        is_leaf = leaf;
        num_keys = 0;
        for (int i = 0; i < 4; i++) {
            children[i] = nullptr;
        }
    }
};
// 2. B-TREE CLASS DEFINITION
class BTree {
private:
    BTreeNode* root;

    void splitChild(BTreeNode* parent, int i, BTreeNode* fullChild) {
        BTreeNode* newNode = new BTreeNode(fullChild->is_leaf);
        newNode->num_keys = 1;

        newNode->keys[0] = fullChild->keys[2];

        if (!fullChild->is_leaf) {
            newNode->children[0] = fullChild->children[2];
            newNode->children[1] = fullChild->children[3];
        }

        fullChild->num_keys = 1;

        for (int j = parent->num_keys; j >= i + 1; j--) {
            parent->children[j + 1] = parent->children[j];
        }
        parent->children[i + 1] = newNode;

        for (int j = parent->num_keys - 1; j >= i; j--) {
            parent->keys[j + 1] = parent->keys[j];
        }

        parent->keys[i] = fullChild->keys[1];
        parent->num_keys++;
    }

    void insertNonFull(BTreeNode* node, Movie k) {
        int i = node->num_keys - 1;

        if (node->is_leaf) {
            while (i >= 0 && node->keys[i].id > k.id) {
                node->keys[i + 1] = node->keys[i];
                i--;
            }
            node->keys[i + 1] = k;
            node->num_keys++;
        } else {
            while (i >= 0 && node->keys[i].id > k.id) {
                i--;
            }
            i++;

            if (node->children[i]->num_keys == 3) {
                splitChild(node, i, node->children[i]);
                if (node->keys[i].id < k.id) {
                    i++;
                }
            }
            insertNonFull(node->children[i], k);
        }
    }

    void inOrderTraversal(BTreeNode* node, vector<Movie>& movies) const {
        if (node != nullptr) {
            int i;
            for (i = 0; i < node->num_keys; i++) {
                if (!node->is_leaf) {
                    inOrderTraversal(node->children[i], movies);
                }
                movies.push_back(node->keys[i]);
            }
            if (!node->is_leaf) {
                inOrderTraversal(node->children[i], movies);
            }
        }
    }

    void freeTree(BTreeNode* node) {
        if (node != nullptr) {
            if (!node->is_leaf) {
                for (int i = 0; i <= node->num_keys; i++) {
                    freeTree(node->children[i]);
                }
            }
            delete node;
        }
    }

public:
    BTree() { root = nullptr; }
    ~BTree() { freeTree(root); }

    void insert(Movie k) {
        if (root == nullptr) {
            root = new BTreeNode(true);
            root->keys[0] = k;
            root->num_keys = 1;
        } else {
            if (root->num_keys == 3) {
                BTreeNode* newRoot = new BTreeNode(false);
                newRoot->children[0] = root;
                splitChild(newRoot, 0, root);

                int i = 0;
                if (newRoot->keys[0].id < k.id) {
                    i++;
                }
                insertNonFull(newRoot->children[i], k);
                root = newRoot;
            } else {
                insertNonFull(root, k);
            }
        }
    }

    Movie* searchById(int id) {
        return searchByIdHelper(root, id);
    }

    Movie* searchByIdHelper(BTreeNode* node, int id) {
        if (node == nullptr) return nullptr;

        int i = 0;
        while (i < node->num_keys && id > node->keys[i].id) {
            i++;
        }

        if (i < node->num_keys && id == node->keys[i].id) {
            return &(node->keys[i]);
        }

        if (node->is_leaf) return nullptr;

        return searchByIdHelper(node->children[i], id);
    }

    vector<Movie> getAllMovies() const {
        vector<Movie> movies;
        inOrderTraversal(root, movies);
        return movies;
    }

    void clear() {
        freeTree(root);
        root = nullptr;
    }
};
// 3. FILE AND UTILITY FUNCTIONS
void printMovie(const Movie& m) {
    cout << "ID: " << setw(4) << left << m.id
         << " | Title: " << setw(20) << left << m.title
         << " | Director: " << setw(15) << left << m.director
         << " | Year: " << setw(5) << left << m.year
         << " | Avg Rating: " << fixed << setprecision(2) << m.getAverageRating() << " [";
    for (size_t i = 0; i < m.ratings.size(); ++i) {
        cout << m.ratings[i] << (i + 1 < m.ratings.size() ? ", " : "");
    }
    cout << "]\n";
}
void loadFromFile(BTree& tree, const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        cout << "File " << filename << " not found. Starting with an empty tree.\n";
        return;
    }

    tree.clear();
    string line;
    int count = 0;
    while (getline(file, line)) {
        if (line.empty()) continue;
        stringstream ss(line);
        string id_str, title, director, year_str, ratings_str;

        if (getline(ss, id_str, ',') &&
            getline(ss, title, ',') &&
            getline(ss, director, ',') &&
            getline(ss, year_str, ',') &&
            getline(ss, ratings_str)) {

            Movie m;
            m.id = stoi(id_str);
            m.title = title;
            m.director = director;
            m.year = stoi(year_str);

            stringstream rss(ratings_str);
            double rating;
            while (rss >> rating) {
                m.ratings.push_back(rating);
            }

            tree.insert(m);
            count++;
        }
    }
    file.close();
    cout << "Successfully loaded " << count << " movies into the tree.\n";
}

void saveToFile(const BTree& tree, const string& filename) {
    ofstream file(filename);
    if (!file.is_open()) {
        cout << "Error opening file for writing!\n";
        return;
    }

    vector<Movie> movies = tree.getAllMovies();
    for (const auto& m : movies) {
        file << m.id << "," << m.title << "," << m.director << "," << m.year;
        for (double r : m.ratings) {
            file << " " << r;
        }
        file << "\n";
    }
    file.close();
    cout << "File updated successfully.\n";
}

void rebuildTree(BTree& tree, const vector<Movie>& movies) {
    tree.clear();
    for (const auto& m : movies) {
        tree.insert(m);
    }
}
// 4. USER OPERATIONS (Search, Filter, Stats)
void searchByDirector(const BTree& tree) {
    cin.ignore();
    cout << "Enter director's last name: ";
    string director;
    getline(cin, director);

    vector<Movie> movies = tree.getAllMovies();
    bool found = false;
    cout << "\n--- Movies directed by: " << director << " ---\n";
    for (const auto& m : movies) {
        if (m.director == director) {
            printMovie(m);
            found = true;
        }
    }
    if (!found) cout << "No movies found for this director: " << director << "\n";
}

void filterBeforeYear(const BTree& tree) {
    int year;
    cout << "Enter threshold year (N): ";
    cin >> year;

    vector<Movie> movies = tree.getAllMovies();
    cout << "\n--- Movies released BEFORE " << year << " ---\n";
    bool found = false;
    for (const auto& m : movies) {
        if (m.year < year) {
            printMovie(m);
            found = true;
        }
    }
    if (!found) cout << "No movies found released before year " << year << ".\n";
}

void filterByDirectorAndPeriod(const BTree& tree) {
    cin.ignore();
    cout << "Enter director's last name: ";
    string director;
    getline(cin, director);

    int year1, year2;
    cout << "Enter start year (Year1): ";
    cin >> year1;
    cout << "Enter end year (Year2): ";
    cin >> year2;

    vector<Movie> movies = tree.getAllMovies();
    cout << "\n--- Director " << director << "'s movies from " << year1 << " to " << year2 << " ---\n";
    bool found = false;
    for (const auto& m : movies) {
        if (m.director == director && m.year >= year1 && m.year <= year2) {
            printMovie(m);
            found = true;
        }
    }
    if (!found) cout << "No movies found matching the criteria.\n";
}

void showStatisticsAndAnalysis(const BTree& tree) {
    vector<Movie> movies = tree.getAllMovies();
    if (movies.empty()) {
        cout << "Catalog is empty.\n";
        return;
    }

    cout << "\n================ STATISTICS AND ANALYSIS ================\n";
    
    // 1. Count films of a director with rating higher than N
    cin.ignore();
    string dir;
    double threshold;
    cout << "Enter director's name for rating analysis: ";
    getline(cin, dir);
    cout << "Enter minimum rating threshold (N): ";
    cin >> threshold;

    int countHigher = 0;
    for (const auto& m : movies) {
        if (m.director == dir && m.getAverageRating() > threshold) {
            countHigher++;
        }
    }
    cout << "Number of movies by " << dir << " with average rating > " << threshold << ": " << countHigher << "\n";

    // 2. Highest and lowest rated movies
    Movie highest = movies[0];
    Movie lowest = movies[0];
    for (const auto& m : movies) {
        if (m.getAverageRating() > highest.getAverageRating()) {
            highest = m;
        }
        if (m.getAverageRating() < lowest.getAverageRating()) {
            lowest = m;
        }
    }

    cout << "\n[Highest Rated Movie]:\n";
    printMovie(highest);
    cout << "\n[Lowest Rated Movie]:\n";
    printMovie(lowest);
}

void removeMovie(BTree& tree, const string& filename) {
    cout << "Delete by: (1) ID or (2) Director? Choose option: ";
    int choice;
    cin >> choice;

    vector<Movie> movies = tree.getAllMovies();
    vector<Movie> updatedMovies;
    int removedCount = 0;

    if (choice == 1) {
        int id;
        cout << "Enter ID of the movie to remove: ";
        cin >> id;
        for (const auto& m : movies) {
            if (m.id == id) {
                removedCount++;
            } else {
                updatedMovies.push_back(m);
            }
        }
    } else if (choice == 2) {
        cin.ignore();
        cout << "Enter director's last name: ";
        string dir;
        getline(cin, dir);
        for (const auto& m : movies) {
            if (m.director == dir) {
                removedCount++;
            } else {
                updatedMovies.push_back(m);
            }
        }
    } else {
        cout << "Invalid choice.\n";
        return;
    }

    if (removedCount > 0) {
        rebuildTree(tree, updatedMovies);
        saveToFile(tree, filename);
        cout << "Successfully removed " << removedCount << " movie(s).\n";
    } else {
        cout << "No matching movies found for removal.\n";
    }
}

void addNewMovie(BTree& tree, const string& filename) {
    Movie m;
    cout << "Enter movie ID: ";
    cin >> m.id;

    if (tree.searchById(m.id) != nullptr) {
        cout << "Error: Movie with ID " << m.id << " already exists!\n";
        return;
    }

    cin.ignore();
    cout << "Enter title: ";
    getline(cin, m.title);
    cout << "Enter director: ";
    getline(cin, m.director);
    cout << "Enter release year: ";
    cin >> m.year;

    int numRatings;
    cout << "How many ratings would you like to enter? ";
    cin >> numRatings;
    cout << "Enter " << numRatings << " ratings (separated by spaces): ";
    for (int i = 0; i < numRatings; ++i) {
        double r;
        cin >> r;
        m.ratings.push_back(r);
    }

    tree.insert(m);
    saveToFile(tree, filename);
    cout << "Movie added successfully!\n";
}
// 5. MAIN MENU LOOP
int main() {
    BTree tree;
    string filename = "movies.txt";

    loadFromFile(tree, filename);

    int choice;
    do {
        cout << "\n================ MOVIE ANALYSIS (B-TREE M=4) ================\n";
        cout << "1. Search by ID\n";
        cout << "2. Search by Director\n";
        cout << "3. All Movies (Sorted in descending order by ID)\n";
        cout << "4. Filter: Movies released before year N\n";
        cout << "5. Filter: By director and period (From Year1 to Year2)\n";
        cout << "6. Statistics and Analysis (Highest/lowest, director ratings)\n";
        cout << "7. Add new movie\n";
        cout << "8. Remove movie (by ID or director)\n";
        cout << "0. Exit\n";
        cout << "Choose an option: ";
        cin >> choice;

        switch (choice) {
            case 1: {
                int id;
                cout << "Enter ID: ";
                cin >> id;
                Movie* m = tree.searchById(id);
                if (m) {
                    cout << "Movie found:\n";
                    printMovie(*m);
                } else {
                    cout << "Movie with ID " << id << " not found.\n";
                }
                break;
            }
            case 2:
                searchByDirector(tree);
                break;
            case 3: {
                vector<Movie> movies = tree.getAllMovies();
                reverse(movies.begin(), movies.end());
                cout << "\n--- Movies in descending order by ID ---\n";
                for (const auto& m : movies) {
                    printMovie(m);
                }
                break;
            }
            case 4:
                filterBeforeYear(tree);
                break;
            case 5:
                filterByDirectorAndPeriod(tree);
                break;
            case 6:
                showStatisticsAndAnalysis(tree);
                break;
            case 7:
                addNewMovie(tree, filename);
                break;
            case 8:
                removeMovie(tree, filename);
                break;
            case 0:
                cout << "Exiting program.\n";
                break;
            default:
                cout << "Invalid choice. Please try again.\n";
        }
    } while (choice != 0);

    return 0;
}