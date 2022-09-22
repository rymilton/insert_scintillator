# insert_scintillator
To run this, do the following:
  1. `git clone https://github.com/rymilton/insert_scintillator.git && cd insert_scintillator`
  2. `mkdir build && cd build`
  3. `cmake ..`
  4. `make`
  5. `./insert_scintillator`

To-do list:
  - Add optical parameters for materials
  - Add correct materials for ESR film, SiPM, and reflective paint
  - Implement RunAction (including output file), EventAction, and SensitiveDetector
