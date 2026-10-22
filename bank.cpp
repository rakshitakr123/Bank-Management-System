
#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <sstream>
#include <iomanip>
#include <limits>
#include <cmath>

using namespace std;

class BankAccount
{
private:
    string bankName = "TCBOI";
    string name, accountNumber, password;
    string aadhaar, email, phone;
    double balance = 0.0;
    vector<string> transactionHistory;

    string readText(const string &prompt)
    {
        string value;
        cout << prompt;
        getline(cin, value);
        return value;
    }

    int readChoice(const string &prompt)
    {
        while (true)
        {
            string input;
            cout << prompt;
            getline(cin, input);

            stringstream ss(input);
            int choice;
            char extra;

            if (ss >> choice && !(ss >> extra))
                return choice;

            cout << "Enter a valid number.\n";
        }
    }

    double readAmount(const string &prompt)
    {
        while (true)
        {
            string input;
            cout << prompt;
            getline(cin, input);

            stringstream ss(input);
            double amount;
            char extra;

            if (ss >> amount && !(ss >> extra) &&
                isfinite(amount) && amount > 0 &&
                amount <= 1000000000.0)
            {
                return round(amount * 100.0) / 100.0;
            }

            cout << "Enter a valid positive amount.\n";
        }
    }

    void pauseScreen()
    {
        cout << "\nPress Enter to continue...";
        string temp;
        getline(cin, temp);
    }

    bool saveToFile()
    {
        ofstream file(accountNumber + ".txt");

        if (!file)
        {
            cout << "Error saving account!\n";
            return false;
        }

        file << name << '\n';
        file << accountNumber << '\n';
        file << password << '\n';
        file << aadhaar << '\n';
        file << email << '\n';
        file << phone << '\n';
        file << fixed << setprecision(2) << balance << '\n';

        for (const string &transaction : transactionHistory)
        {
            file << transaction << '\n';
        }

        file.close();
        return !file.fail();
    }

    bool loadFromFile()
    {
        ifstream file(accountNumber + ".txt");

        if (!file)
            return false;

        string savedNumber, balanceLine;

        if (!getline(file, name) ||
            !getline(file, savedNumber) ||
            !getline(file, password) ||
            !getline(file, aadhaar) ||
            !getline(file, email) ||
            !getline(file, phone) ||
            !getline(file, balanceLine))
        {
            return false;
        }

        stringstream ss(balanceLine);
        char extra;

        if (!(ss >> balance) || (ss >> extra) ||
            !isfinite(balance) || balance < 0 ||
            savedNumber != accountNumber)
        {
            return false;
        }

        transactionHistory.clear();

        string transaction;
        while (getline(file, transaction))
        {
            transactionHistory.push_back(transaction);
        }

        return true;
    }

    void addTransaction(const string &details)
    {
        transactionHistory.push_back(details);
    }

public:
    void createAccount()
    {
        cout << "\n========== CREATE ACCOUNT ==========\n";

        name = readText("Full name: ");
        email = readText("Email: ");
        phone = readText("Phone number: ");
        aadhaar = readText("Aadhaar number: ");
        password = readText("Create password: ");

        if (name.empty() || email.empty() ||
            phone.empty() || aadhaar.empty() ||
            password.empty())
        {
            cout << "All fields are required.\n";
            return;
        }

        int lastNumber = 1000;
        ifstream input("account_counter.txt");

        if (input && !(input >> lastNumber))
        {
            cout << "Invalid account counter file.\n";
            return;
        }
        input.close();

        if (lastNumber < 1000 ||
            lastNumber >= numeric_limits<int>::max())
        {
            cout << "Invalid account counter.\n";
            return;
        }

        int nextNumber = lastNumber + 1;
        accountNumber = bankName + to_string(nextNumber);

        ifstream existing(accountNumber + ".txt");
        if (existing)
        {
            cout << "Account already exists. Check account counter.\n";
            return;
        }
        existing.close();

        balance = 500.0;
        transactionHistory.clear();
        addTransaction("Opening balance: Rs. 500.00");

        if (!saveToFile())
        {
            cout << "Account could not be saved.\n";
            return;
        }

        ofstream output("account_counter.txt");
        if (!output)
        {
            cout << "Could not save account counter.\n";
            return;
        }

        output << nextNumber;
        output.close();

        if (!output)
        {
            cout << "Could not update account counter.\n";
            return;
        }

        cout << "\nAccount created successfully!\n";
        cout << "Account number: " << accountNumber << '\n';
        cout << "Initial balance: Rs. 500.00\n";
        cout << "Remember your account number and password.\n";
    }

    void loginBankAccount()
    {
        cout << "\n========== LOGIN ==========\n";

        string enteredNumber = readText("Account number: ");

        if (enteredNumber.empty())
        {
            cout << "Account number cannot be empty.\n";
            return;
        }

        accountNumber = enteredNumber;

        if (!loadFromFile())
        {
            cout << "Account not found or file is invalid.\n";
            return;
        }

        string enteredPassword = readText("Password: ");

        if (enteredPassword != password)
        {
            cout << "Incorrect password!\n";
            return;
        }

        cout << "Login successful! Welcome, " << name << ".\n";
        mainMenu();
    }

    void depositMoney()
    {
        double amount = readAmount("Amount to deposit: Rs. ");

        if (balance > 1000000000.0 - amount)
        {
            cout << "Balance limit exceeded.\n";
            return;
        }

        double oldBalance = balance;
        size_t oldSize = transactionHistory.size();

        balance += amount;

        ostringstream record;
        record << fixed << setprecision(2)
               << "Deposit: Rs. " << amount;
        addTransaction(record.str());

        if (saveToFile())
        {
            cout << "Deposit successful!\n";
            checkBalance();
        }
        else
        {
            balance = oldBalance;
            transactionHistory.resize(oldSize);
            cout << "Could not save deposit.\n";
        }
    }

    void withdrawMoney()
    {
        double amount = readAmount("Amount to withdraw: Rs. ");

        if (amount > balance)
        {
            cout << "Insufficient balance!\n";
            return;
        }

        double oldBalance = balance;
        size_t oldSize = transactionHistory.size();

        balance -= amount;

        ostringstream record;
        record << fixed << setprecision(2)
               << "Withdrawal: Rs. " << amount;
        addTransaction(record.str());

        if (saveToFile())
        {
            cout << "Withdrawal successful!\n";
            checkBalance();
        }
        else
        {
            balance = oldBalance;
            transactionHistory.resize(oldSize);
            cout << "Could not save withdrawal.\n";
        }
    }

    // NEW FEATURE 1: Transfer money to another account.
    void transferMoney()
    {
        cout << "\n========== MONEY TRANSFER ==========\n";

        string receiverNumber =
            readText("Receiver account number: ");

        if (receiverNumber.empty() ||
            receiverNumber == accountNumber)
        {
            cout << "Invalid receiver account.\n";
            return;
        }

        BankAccount receiver;
        receiver.accountNumber = receiverNumber;

        if (!receiver.loadFromFile())
        {
            cout << "Receiver account not found.\n";
            return;
        }

        cout << "Receiver name: " << receiver.name << '\n';

        double amount = readAmount("Amount to transfer: Rs. ");

        if (amount > balance)
        {
            cout << "Insufficient balance!\n";
            return;
        }

        if (receiver.balance > 1000000000.0 - amount)
        {
            cout << "Receiver balance limit exceeded.\n";
            return;
        }

        string confirmation = readText("Confirm transfer (yes/no): ");

        if (confirmation != "yes" && confirmation != "YES")
        {
            cout << "Transfer cancelled.\n";
            return;
        }

        double oldSenderBalance = balance;
        double oldReceiverBalance = receiver.balance;
        size_t oldSenderHistory = transactionHistory.size();
        size_t oldReceiverHistory = receiver.transactionHistory.size();

        balance -= amount;
        receiver.balance += amount;

        ostringstream outgoing, incoming;
        outgoing << fixed << setprecision(2)
                 << "Transfer to " << receiverNumber
                 << ": Rs. " << amount;

        incoming << fixed << setprecision(2)
                 << "Transfer from " << accountNumber
                 << ": Rs. " << amount;

        addTransaction(outgoing.str());
        receiver.addTransaction(incoming.str());

        // Save both accounts.
        if (receiver.saveToFile() && saveToFile())
        {
            cout << "Transfer successful!\n";
            checkBalance();
        }
        else
        {
            // Restore the original in-memory values.
            balance = oldSenderBalance;
            receiver.balance = oldReceiverBalance;
            transactionHistory.resize(oldSenderHistory);
            receiver.transactionHistory.resize(oldReceiverHistory);

            saveToFile();
            receiver.saveToFile();

            cout << "Transfer failed. Previous values restored if saving succeeds.\n";
        }
    }

    void checkBalance()
    {
        cout << fixed << setprecision(2);
        cout << "Current balance: Rs. " << balance << '\n';
    }

    void accountDetails()
    {
        cout << "\n========== ACCOUNT DETAILS ==========\n";
        cout << "Bank: " << bankName << '\n';
        cout << "Account holder: " << name << '\n';
        cout << "Account number: " << accountNumber << '\n';
        cout << "Email: " << email << '\n';
        cout << "Phone: " << phone << '\n';
        cout << "Aadhaar: " << aadhaar << '\n';
        checkBalance();
    }

    // NEW FEATURE 2: Show all saved transactions.
    void allTransactionHistory()
    {
        cout << "\n========== TRANSACTION HISTORY ==========\n";

        if (transactionHistory.empty())
        {
            cout << "No transactions available.\n";
            return;
        }

        for (size_t i = 0; i < transactionHistory.size(); ++i)
        {
            cout << i + 1 << ". "
                 << transactionHistory[i] << '\n';
        }
    }

    void mainMenu()
    {
        while (true)
        {
            cout << "\n========== ACCOUNT MENU ==========\n";
            cout << "1. Deposit money\n";
            cout << "2. Withdraw money\n";
            cout << "3. Transfer money\n";
            cout << "4. Check balance\n";
            cout << "5. Account details\n";
            cout << "6. Transaction history\n";
            cout << "7. Logout\n";

            int choice = readChoice("Enter choice: ");

            switch (choice)
            {
            case 1:
                depositMoney();
                break;
            case 2:
                withdrawMoney();
                break;
            case 3:
                transferMoney();
                break;
            case 4:
                checkBalance();
                break;
            case 5:
                accountDetails();
                break;
            case 6:
                allTransactionHistory();
                break;
            case 7:
                cout << "Logged out successfully.\n";
                return;
            default:
                cout << "Invalid choice.\n";
            }

            pauseScreen();
        }
    }
};

int main()
{
    BankAccount account;

    while (true)
    {
        cout << "\n====================================\n";
        cout << "          TCBOI BANK\n";
        cout << "====================================\n";
        cout << "1. Create account\n";
        cout << "2. Login\n";
        cout << "3. Exit\n";

        string input;
        cout << "Enter choice: ";
        getline(cin, input);

        stringstream ss(input);
        int choice;
        char extra;

        if (!(ss >> choice) || (ss >> extra))
        {
            cout << "Enter a valid number.\n";
            continue;
        }

        switch (choice)
        {
        case 1:
            account.createAccount();
            break;
        case 2:
            account.loginBankAccount();
            break;
        case 3:
            cout << "Thank you for using TCBOI Bank!\n";
            return 0;
        default:
            cout << "Invalid choice.\n";
        }
    }
}
