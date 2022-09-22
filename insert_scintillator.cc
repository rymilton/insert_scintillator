#include "DetectorConstruction.hh"
#include "ActionInitialization.hh"

#include "G4RunManagerFactory.hh"
#include "G4SteppingVerbose.hh"
#include "G4UImanager.hh"


#include "G4VisExecutive.hh"
#include "G4UIExecutive.hh"
#include "FTFP_BERT.hh"
#include "G4EmStandardPhysics_option4.hh"
#include "G4OpticalPhysics.hh"

#include "Randomize.hh"

using namespace InsertScintillator;

void PrintUsage()
{
    G4cerr << " Usage: " << G4endl;
    G4cerr << " insert_scintillator [-m macro ] [-t nThreads]" << G4endl;
    G4cerr << " note: -t option is available only for multi-threaded mode." << G4endl;
}

int main(int argc, char **argv)
{
    if (argc > 5)
    {
        PrintUsage();
        return 1;
    }
    G4String macro;

#ifdef G4MULTITHREADED
    G4int multi_threaded = 1;
    G4int nThreads = 0;
#endif

    // Getting the options from the command line
    for (G4int i = 1; i < argc; i += 2)
    {
        if (G4String(argv[i]) == "-m")
            macro = argv[i + 1];
        else if (multi_threaded && G4String(argv[i]) == "-t")
        {
            nThreads = G4UIcommand::ConvertToInt(argv[i + 1]);
        }
        else
        {
            PrintUsage();
            return 1;
        }
    }

    // Detect interactive mode (if no arguments) and define UI session
    G4UIExecutive *ui = nullptr;

    if (!macro.size())
    {
        ui = new G4UIExecutive(argc, argv);
    }

    // use G4SteppingVerboseWithUnits
    G4int precision = 4;
    G4SteppingVerbose::UseBestUnit(precision);

    // Construct the default run manager
    auto *runManager =
        G4RunManagerFactory::CreateRunManager(G4RunManagerType::Default);
    if (multi_threaded && nThreads > 0)
    {
        runManager->SetNumberOfThreads(nThreads);
    }

    // Set mandatory initialization classes

    // Detector construction
    runManager->SetUserInitialization(new DetectorConstruction());

    // Physics list
    G4VModularPhysicsList* physicsList = new FTFP_BERT;
    physicsList->ReplacePhysics(new G4EmStandardPhysics_option4());
    G4OpticalPhysics* opticalPhysics = new G4OpticalPhysics();

    physicsList->RegisterPhysics(opticalPhysics);
    runManager->SetUserInitialization(physicsList);

    // User action initialization
    runManager->SetUserInitialization(new ActionInitialization());
    
    // Initialize visualization
    G4VisManager *visManager = new G4VisExecutive;
    visManager->Initialize();

    // Get the pointer to the User Interface manager
    G4UImanager *UImanager = G4UImanager::GetUIpointer();

    // Process macro or start UI session
    if (macro.size())
    {
        // batch mode
        G4String command = "/control/execute ";
        UImanager->ApplyCommand(command + macro);
    }
    else
    {
        // interactive mode
        UImanager->ApplyCommand("/control/execute init_vis.mac");
        ui->SessionStart();
        delete ui;
    }

    // Job termination
    // Free the store: user actions, physics_list and detector_description are
    // owned and deleted by the run manager, so they should not be deleted
    // in the main() program !

    delete visManager;
    delete runManager;

    return 0;
}