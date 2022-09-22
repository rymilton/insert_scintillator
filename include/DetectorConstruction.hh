#ifndef DetectorConstruction_h
#define DetectorConstruction_h 1

#include "G4VUserDetectorConstruction.hh"
#include "G4VPhysicalVolume.hh"
#include "G4LogicalVolume.hh"

#include "G4RunManager.hh"
#include "G4NistManager.hh"
#include "G4Box.hh"
#include "G4SystemOfUnits.hh"
#include "G4PVPlacement.hh"

namespace InsertScintillator
{

  class DetectorConstruction : public G4VUserDetectorConstruction
  {
  public:
    DetectorConstruction();
    ~DetectorConstruction() override;

    G4VPhysicalVolume *Construct() override;
    void DefineMaterials();
    G4VPhysicalVolume *DefineVolumes();
    void ConstructSDandField() override;
  };

}

#endif