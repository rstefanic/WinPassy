# WinPassy
A Windows password manager that you can manage from the command line. I typically found that a lot of people around me were just storing their passwords in plain text. So I figured that people could store their passwords in a slightly more secure manner. It ain't the best solution, but it sure beats storing your passwords in a plain text file.

## Features
- Have one master password to manage all of your other passwords
- Store virtually as many services with account names and passwords as you'd like
- Your master password is hashed using SHA 512
- Your individual service passwords are encrypted with your master password using SHA 256
- Each user can manage their own set of passwords
- Your encrypted services are stored in your user folder (e.g. "C:\Users\<UserName>") for ease of access and backup

## Getting started
Once you've cloned the repo, you can use `build.bat` to build the project if you have clang installed. This will output a file in a local `./bin` directory with an exe that you can run (or move it someone more convenient such as a folder in your $PATH).

If you plan on adding anything or modifying the source code, there is a `DEBUG` flag in `inc/winpassy.h` that can set to `1` for debugging.

## Contributing
Feel free to make a contribution to the project and submit a pull request.
