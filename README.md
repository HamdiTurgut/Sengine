Sengine
A lightweight C++ application search engine simulator that queries a local database to find and launch software.

Overview
Sengine is a command-line utility built in C++ that simulates the core logic of a search engine. It compares user input against a hardcoded database of 100 popular software applications, ranging from development tools to video games.

Features
Typo Tolerance: The algorithm calculates a match rate by comparing the input and the database character by character. If the user makes a single-character mistake, the engine still recognizes the input and triggers the application.

Fast Length Filtering: To optimize the search process, the system immediately skips any database entry that does not have the exact same character length as the user's search query.

Array-Based Storage: Utilizes a standard vector structure to store the application database, making it easy to read, expand, and maintain.
