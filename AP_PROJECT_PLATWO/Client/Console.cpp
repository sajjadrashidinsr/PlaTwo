#include "Console.h"

#include <iostream>

using namespace std;

int Console::showMenu()
{
    int choice;

    cout << "\n========================\n";
    cout << "1.Login\n";
    cout << "2.Sign Up\n";
    cout << "3.Forgot Password\n";
    cout << "0.Exit\n";

    cout << "\nChoice : ";

    cin >> choice;

    cin.ignore();

    return choice;
}

User Console::getSignupInformation()
{
    User user;

    string input;

    cout << "Name : ";
    getline(cin, input);
    user.setName(input);

    cout << "Username : ";
    getline(cin, input);
    user.setUsername(input);

    cout << "Password : ";
    getline(cin, input);
    user.setPasswordHash(input);

    cout << "Phone : ";
    getline(cin, input);
    user.setPhone(input);

    cout << "Email : ";
    getline(cin, input);
    user.setEmail(input);

    return user;
}

User Console::getLoginInformation()
{
    User user;

    string input;

    cout << "Username : ";
    getline(cin, input);
    user.setUsername(input);

    cout << "Password : ";
    getline(cin, input);
    user.setPasswordHash(input);

    return user;
}

string Console::getPhone()
{
    string phone;

    cout << "Phone : ";

    getline(cin, phone);

    return phone;
}

string Console::getNewPassword()
{
    string password;

    cout << "New Password : ";

    getline(cin, password);

    return password;
}

void Console::showMessage(const string& message)
{
    cout << endl;

    cout << message << endl;

    cout << endl;
}