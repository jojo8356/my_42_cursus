# Get Next Line — Reading a line from a file descriptor is way too tedious

> **Official 42 Curriculum Subject** — **Version 14.3** (2025–2026 Common Core)  
> **Summary:** *This activity is about programming a function that returns a line read from a file descriptor.*

---

## Contents

- [Chapter I — Goals](#chapter-i--goals)
- [Chapter II — Common Instructions](#chapter-ii--common-instructions)
- [Chapter III — AI Instructions](#chapter-iii--ai-instructions)
- [Chapter IV — Mandatory Part](#chapter-iv--mandatory-part)
- [Chapter V — Readme Requirements](#chapter-v--readme-requirements)
- [Chapter VI — Bonus Part](#chapter-vi--bonus-part)
- [Chapter VII — Submission and Peer-Evaluation](#chapter-vii--submission-and-peer-evaluation)

---

## Chapter I — Goals

This activity will not only allow you to add a highly useful function to your collection, but it will also teach you an important concept in C programming: **static variables**.

---

## Chapter II — Common Instructions

- Your project must be written in **C**.
- Your project must be written in accordance with the **Norm**. If you have bonus files/functions, they are included in the norm check, and you will receive a **0** if there is a norm error.
- Your functions should not quit unexpectedly (*segmentation fault*, *bus error*, *double free*, etc.) except for undefined behavior. If this occurs, your project will be considered non-functional and will receive a **0** during the evaluation.
- All heap-allocated memory must be properly freed when necessary. **Memory leaks will not be tolerated.**
- If the subject requires it, you must submit a `Makefile` that compiles your source files to the required output with the flags `-Wall`, `-Wextra`, and `-Werror`, using `cc`. Additionally, your `Makefile` must not perform unnecessary relinking.
- Your `Makefile` must contain at least the rules `$(NAME)`, `all`, `clean`, `fclean` and `re`.
- To submit bonuses for your project, you must include a `bonus` rule in your `Makefile`, which will add all the various headers, libraries, or functions that are not allowed in the main part of the project. Bonuses must be placed in `_bonus.{c/h}` files, unless the subject specifies otherwise. The evaluation of mandatory and bonus parts is conducted separately.
- If your project allows you to use your `libft`, you must copy its sources and its associated `Makefile` into a `libft` folder. Your project's `Makefile` must compile the library by using its `Makefile`, then compile the project.
- We encourage you to create test programs for your project, even though this work **does not need to be submitted and will not be graded**. It will give you an opportunity to easily test your work and your peers' work. You will find these tests especially useful during your defence. Indeed, during defence, you are free to use your tests and/or the tests of the peer you are evaluating.
- Submit your work to the assigned Git repository. Only the work in the Git repository will be graded. If *Deepthought* is assigned to grade your work, it will occur after your peer-evaluations. If an error happens in any section of your work during *Deepthought*'s grading, the evaluation will stop.

---

## Chapter III — AI Instructions

### Context
This project is designed to help you discover the fundamental building blocks of your 42 training. To properly anchor key knowledge and skills, it is essential to adopt a thoughtful approach to using AI tools and support. True foundational learning requires genuine intellectual effort — through challenge, repetition, and peer-learning exchanges.

### Main Message
- Build strong foundations without shortcuts.
- Really develop tech & power skills.
- Experience real peer-learning, start learning how to learn and solve new problems.
- The learning journey is more important than the result.
- Learn about the risks associated with AI, and develop effective control practices and countermeasures to avoid common pitfalls.

### Learner Rules
- You should apply reasoning to your assigned tasks, especially before turning to AI.
- You should not ask for direct answers to the AI.
- You should learn about 42's global approach on AI.

### Phase Outcomes
- Get proper tech and coding foundations.
- Know why and how AI can be dangerous during this phase.

---

## Chapter IV — Mandatory Part

| Field | Specification |
| :--- | :--- |
| **Function Name** | `get_next_line` |
| **Prototype** | `char *get_next_line(int fd);` |
| **Files to Submit** | `get_next_line.c`, `get_next_line_utils.c`, `get_next_line.h`, **all at the root of the repository** |
| **Parameters** | `fd`: The file descriptor to read from |
| **Return Value** | **Read line**: correct behavior<br>**`NULL`**: there is nothing else to read, or an error occurred |
| **External Functions** | `read`, `malloc`, `free` |
| **Description** | Write a function that returns a line read from a file descriptor |

### Specifications

- Repeated calls (e.g., using a loop) to your `get_next_line()` function should let you read the text file pointed to by the file descriptor, **one line at a time**.
- Your function should return the line that was read. If there is nothing left to read or if an error occurs, it should return `NULL`.
- Make sure that your function works as expected both when reading a file and when reading from the **standard input**.
- **Please note** that the returned line should include the terminating `\n` character, except when the end of the file is reached and the file does not end with a `\n` character.
- Your header file `get_next_line.h` must at least contain the prototype of the `get_next_line()` function.
- Add all the helper functions you need in the `get_next_line_utils.c` file.

> *A good start would be to know what a static variable is.*

- Because you will have to read files in `get_next_line()`, add this option to your compiler call: `-D BUFFER_SIZE=n`  
  It will define the buffer size for `read()`.  
  The buffer size value will be adjusted by your peer evaluators and the Moulinette to test your code.

> **We must be able to compile this project with and without the `-D BUFFER_SIZE` flag in addition to the usual flags. You may choose any default value you prefer.**

- You will compile your code as follows (a buffer size of `42` is used as an example):
  ```bash
  cc -Wall -Wextra -Werror -D BUFFER_SIZE=42 <files>.c
  ```
  *(Note: the files must be at the root of the repository).*
- `get_next_line()` exhibits undefined behavior if the file associated with the file descriptor is modified after the last call, while `read()` has not yet reached the end of the file.
- `get_next_line()` also exhibits undefined behavior when reading a binary file. However, you can implement a logical way to handle this behavior if you want to.

> *Does your function still work if the `BUFFER_SIZE` value is `9999`? If it is `1`? `10000000`? Do you know why?*

> **Try to read as little as possible each time `get_next_line()` is called. If you encounter a new line, you have to return the current line. Don't read the whole file and then process each line.**

### Forbidden
- You are **not allowed** to use your `libft` in this project.
- `lseek()` is **forbidden**.
- **Global variables** are **forbidden**.

---

## Chapter V — Readme Requirements

A `README.md` file must be provided at the **root of your Git repository**. Its purpose is to allow anyone unfamiliar with the project (peers, staff, recruiters, etc.) to quickly understand what the project is about, how to run it, and where to find more information on the topic.

The `README.md` must include at least:
- The very first line must be italicized and read:  
  `*This project has been created as part of the 42 curriculum by <login1>[, <login2>[, ...]].*`
- A **"Description"** section that clearly presents the project, including its goal and a brief overview.
- An **"Instructions"** section containing any relevant information about compilation, installation, and/or execution.
- A **"Resources"** section listing classic references related to the topic (documentation, articles, tutorials, etc.), as well as a description of how AI was used — specifying for which tasks and which parts of the project.
- **Project-specific required addition:**  
  A **detailed explanation and justification of the algorithm selected** for this project must also be included.

---

## Chapter VI — Bonus Part

This project is straightforward and does not support complex bonus features. However, we trust your creativity. If you have completed the mandatory part, consider attempting this bonus section.

Here are the bonus part requirements:
- **Develop `get_next_line()` using only one static variable.**
- **Your `get_next_line()` can manage multiple file descriptors at the same time.**  
  For example, if you are reading from file descriptors `3`, `4`, and `5`, you should be able to read from a different file descriptor with each call, without losing track of the reading state of each file descriptor or returning a line from a different one.  
  This means you should be able to call `get_next_line()` to read from `fd 3`, then `fd 4`, then `fd 5`, then again from `fd 3`, then `fd 4`, and so forth, without losing track of the reading state for each file descriptor.

Append the `_bonus.[c/h]` suffix to the bonus part files. In addition to the mandatory part files, you will turn in the **3 following files at the root of the repository**:
- `get_next_line_bonus.c`
- `get_next_line_bonus.h`
- `get_next_line_utils_bonus.c`

> *The bonus part will only be assessed if the mandatory part is perfect. "Perfect" means the mandatory part has been integrally done and works without malfunctioning. If you have not passed ALL the mandatory requirements, your bonus part will not be evaluated at all.*

---

## Chapter VII — Submission and Peer-Evaluation

Submit your assignment in your Git repository as usual. Only the content inside your repository will be evaluated during the defense. Make sure to double-check the file names for accuracy.

When writing your tests, remember that:
1. Both the **buffer size** and the **line size** can be of very different values.
2. A **file descriptor** does not only point to regular files (pipes, `stdin`, sockets, FIFOs, etc.).

Once passed, do not hesitate to add your `get_next_line()` to your `libft`.

During the evaluation, a brief **modification of the project** may occasionally be requested to verify your actual understanding of a specific part of the project.
