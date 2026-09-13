**Sengine: Local Application Search & Execution Simulator**

**Overview**
Sengine is a lightweight command-line search engine built in C++ that simulates querying a local database to find and "launch" software. Designed to operate entirely within the terminal, the system focuses on string manipulation and basic algorithmic matching to handle user inputs.

In software development, handling user input gracefully is critical. Sengine proactively analyzes the length and character composition of a search query, matching it against a predefined list of 100 popular applications to simulate a resilient software launcher.

**Key Features**

* **Local Vector Database:** Securely stores application names in a standard C++ `vector`, allowing for fast indexing, clear readability, and easy expansion of the software catalog.


* **Typo-Tolerant Matching:** The core engine converts strings to C-style character arrays and compares them index-by-index. If the user makes a single-character mistake, the `rate` variable falls exactly one point short of a perfect match, triggering a fail-safe that still executes the application.


* **Length-Based Filtering:** To optimize processing time, the algorithm uses a rapid first-pass filter that immediately skips any database entry that does not exactly match the character length of the search term.


* **Simulated Execution Environment:** Designed for rapid terminal use, it immediately triggers a modular `OpenApp()` function providing visual feedback ("App Opened") when a successful match is identified.



**Use Case**
This system is designed as a foundational mechanism for implementing search algorithms and string manipulation. The primary objective is to solve the operational bottleneck of strict, unforgiving query matching. By capturing the user's input, the software takes on the responsibility of checking the string character by character, ensuring that minor spelling errors do not prevent the user from reaching their intended result. Because generating knowledge and fully digesting how logic is built is more valuable than memorizing syntax, this architecture serves as a clear blueprint for how backend mechanics translate directly into user-facing features.

**Technical Architecture**

* **Frontend:** C++ Standard Output (`std::cout` / Terminal UI)


* **Backend:** C++ (Core loop logic, C-style string conversions)


* **Database:** In-memory `std::vector`

* <img width="1408" height="768" alt="conrolmap" src="https://github.com/user-attachments/assets/522523d0-ade1-45c7-8e90-d5fe4d9577bc" />

