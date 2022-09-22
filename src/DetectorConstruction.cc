#include "DetectorConstruction.hh"
#include "G4ExtrudedSolid.hh"
#include "G4VisAttributes.hh"
#include "G4Colour.hh"
#include "G4Tubs.hh"
#include "G4SubtractionSolid.hh"
#include "G4Sphere.hh"
#include "G4MultiUnion.hh"
#include "G4Material.hh"

namespace InsertScintillator
{
  DetectorConstruction::DetectorConstruction()
  {
  }

  DetectorConstruction::~DetectorConstruction()
  {
  }

  G4VPhysicalVolume *DetectorConstruction::Construct()
  {
    DefineMaterials();
    return DefineVolumes();
  }

  void DetectorConstruction::DefineMaterials()
  {
    auto nistManager = G4NistManager::Instance();
    nistManager->FindOrBuildMaterial("G4_AIR");
    nistManager->FindOrBuildMaterial("G4_PLASTIC_SC_VINYLTOLUENE");
    nistManager->FindOrBuildMaterial("G4_POLYSTYRENE");

    auto Silicon = nistManager->FindOrBuildElement("Si");
    G4Material *Quartz = new G4Material("Quartz", 2.2 * g / cm3, 2);
    Quartz->AddElement(Silicon, 1);
    Quartz->AddElement(nistManager->FindOrBuildElement("O"), 2);

    G4Material *Fr4_Epoxy = new G4Material("Fr4_Epoxy", 1.2 * g / cm3, 2);
    Fr4_Epoxy->AddElement(nistManager->FindOrBuildElement("H"), 2);
    Fr4_Epoxy->AddElement(nistManager->FindOrBuildElement("C"), 2);

    G4Material *Fr4 = new G4Material("Fr4", 1.86 * g / cm3, 2);
    Fr4->AddMaterial(Quartz, 52.8 * perCent);
    Fr4->AddMaterial(Fr4_Epoxy, 47.2 * perCent);
  }
  G4VPhysicalVolume *DetectorConstruction::DefineVolumes()
  {
    G4double scint_zposition = 0 * cm;
    G4double dimple_scaling_factor = 1.35;
    G4double dimple_width = 0.62 * cm * dimple_scaling_factor;
    G4double dimple_radius = 0.38 * cm * dimple_scaling_factor;
    G4double air_gap = 0.1 * mm;
    // Get materials
    auto defaultMaterial = G4Material::GetMaterial("G4_AIR");
    auto scintillatorMaterial = G4Material::GetMaterial("G4_PLASTIC_SC_VINYLTOLUENE");
    auto PCBMaterial = G4Material::GetMaterial("Fr4");
    auto ESRMaterial = G4Material::GetMaterial("G4_POLYSTYRENE");

    // World
    auto worldSolid = new G4Box("worldSolid", 0.5 * m, 0.5 * m, 0.5 * m);
    auto worldLogic = new G4LogicalVolume(worldSolid, defaultMaterial, "worldLogic");
    auto worldPhysical = new G4PVPlacement(0, G4ThreeVector(0., 0., 0.), worldLogic, "worldPhysical", 0, false, 0, true);

    auto polygon_creator = [](G4int num_sides, G4double polygon_angle, G4double polygon_rmax)
    {
      std::vector<G4TwoVector> polygon(num_sides);
      for (G4int i = 0; i < num_sides; i++)
      {
        G4double phi = i * polygon_angle;
        G4double cosphi = std::cos(phi);
        G4double sinphi = std::sin(phi);
        polygon[i].set(polygon_rmax * cosphi, polygon_rmax * sinphi);
      }
      return polygon;
    };

    const G4int scint_num_sides = 6;
    G4double scint_polygon_rmax = 1.74 * cm;
    G4double scint_thickness = 0.3 * cm;

    auto scint_polygon = polygon_creator(scint_num_sides, 2. * M_PI / scint_num_sides, scint_polygon_rmax);
    G4TwoVector offsetA(0, 0), offsetB(0, 0);
    G4double scaleA = 1, scaleB = 1;

    G4double dimple_angle = asin((dimple_width / 2.) / dimple_radius);

    auto scintSolid = new G4ExtrudedSolid("scintSolid", scint_polygon, scint_thickness / 2., offsetA, scaleA, offsetB, scaleB);
    auto DimpleSolid = new G4Sphere("DimpleSolid", 0., dimple_radius, 0., 2. * M_PI, 0., dimple_angle);

    G4double dimple_subtraction_z = scint_thickness / 2. + ((dimple_width / 2.) / tan(dimple_angle));
    auto scintWithDimpleSolid = new G4SubtractionSolid("scintWithDimpleSolid", scintSolid, DimpleSolid, G4Transform3D(CLHEP::HepRotationY(M_PI), CLHEP::Hep3Vector(0., 0., dimple_subtraction_z)));

    auto scintLogic = new G4LogicalVolume(scintWithDimpleSolid, scintillatorMaterial, "scintLogic");

    new G4PVPlacement(0, G4ThreeVector(0., 0., scint_zposition), scintLogic, "scintPhysical", worldLogic, false, 0, true);

    // Reflective paint on edges
    const G4int paint_num_sides = scint_num_sides;
    G4double paint_polygon_rmax = scint_polygon_rmax + 1 * mm;
    G4double paint_thickness = scint_thickness;

    auto paint_polygon = polygon_creator(paint_num_sides, 2. * M_PI / paint_num_sides, paint_polygon_rmax);
    auto paintShape = new G4ExtrudedSolid("paintShape", paint_polygon, paint_thickness / 2., offsetA, scaleA, offsetB, scaleB);
    auto scintForPaintShape = new G4ExtrudedSolid("scintForPaintShape", scint_polygon, scint_thickness, offsetA, scaleA, offsetB, scaleB);

    auto paintSolid = new G4SubtractionSolid("paintSolid", paintShape, scintForPaintShape);
    auto paintLogic = new G4LogicalVolume(paintSolid, scintillatorMaterial, "paintLogic");
    new G4PVPlacement(0, G4ThreeVector(0., 0., scint_zposition), paintLogic, "paintPhysical", worldLogic, false, 0, true);

    // Placing ESR foil
    const G4int ESR_num_sides = scint_num_sides;
    G4double ESR_polygon_rmax = scint_polygon_rmax;
    G4double ESR_thickness = 0.015 * cm;
    G4double ESRFront_zposition = scint_zposition - scint_thickness / 2. - ESR_thickness / 2.;
    G4double ESRBack_zposition = scint_zposition + scint_thickness / 2. + ESR_thickness / 2.;

    auto ESR_polygon = polygon_creator(ESR_num_sides, 2. * M_PI / ESR_num_sides, ESR_polygon_rmax);

    G4VSolid *ESRSolid = new G4ExtrudedSolid("ESRSolid", ESR_polygon, ESR_thickness / 2., offsetA, scaleA, offsetB, scaleB);
    auto ESRFrontLogic = new G4LogicalVolume(ESRSolid, ESRMaterial, "ESRFrontLogic");
    new G4PVPlacement(0, G4ThreeVector(0., 0., ESRFront_zposition), ESRFrontLogic, "ESRFrontPhysical", worldLogic, false, 0, true);

    auto ESRDimpleHoleSolid = new G4Tubs("ESRDimpleHoleSolid", 0., dimple_width / 2., ESR_thickness, 0., 2 * M_PI);
    auto ESRWithHoleSolid = new G4SubtractionSolid("ESRWithHoleSolid", ESRSolid, ESRDimpleHoleSolid);
    auto ESRBackLogic = new G4LogicalVolume(ESRWithHoleSolid, ESRMaterial, "ESRBackLogic");
    new G4PVPlacement(0, G4ThreeVector(0., 0., ESRBack_zposition), ESRBackLogic, "ESRBackPhysical", worldLogic, false, 0, true);

    // Placing SiPM

    G4double PCB_width = 4. * cm;
    G4double PCB_height = 4. * cm;
    G4double PCB_thickness = 0.16 * cm;
    G4double PCB_zposition = ESRBack_zposition + ESR_thickness / 2. + PCB_thickness / 2.;

    G4double SiPM_width = 4.35 * mm;
    G4double SiPM_height = 3.85 * mm;
    G4double SiPM_thickness = 1.45 * mm;
    G4double SiPM_zposition = PCB_zposition - PCB_thickness / 2. - SiPM_thickness / 2.;

    auto PCBSolid = new G4Box("PCBSolid", PCB_width / 2., PCB_height / 2., PCB_thickness / 2.);
    auto PCBLogic = new G4LogicalVolume(PCBSolid, PCBMaterial, "PCBLogic");
    new G4PVPlacement(0, G4ThreeVector(0., 0., PCB_zposition), PCBLogic, "PCBPhysical", worldLogic, false, 0, true);

    auto SiPMSolid = new G4Box("SiPMSolid", SiPM_width / 2., SiPM_height / 2., SiPM_thickness / 2.);
    auto SiPMLogic = new G4LogicalVolume(SiPMSolid, scintillatorMaterial, "SiPMLogic");
    new G4PVPlacement(0, G4ThreeVector(0, 0, SiPM_zposition), SiPMLogic, "SiPMPhysical", worldLogic, false, 0, true);

    G4VisAttributes *CyanVisAtt = new G4VisAttributes(G4Colour::Cyan());
    G4VisAttributes *RedVisAtt = new G4VisAttributes(G4Colour::Red());
    G4VisAttributes *GreenVisAtt = new G4VisAttributes(G4Colour::Green());
    G4VisAttributes *BlueVisAtt = new G4VisAttributes(G4Colour::Blue());
    G4VisAttributes *WhiteVisAtt = new G4VisAttributes(G4Colour::White());
    G4VisAttributes *GrayVisAtt = new G4VisAttributes(G4Colour::Gray());
    G4VisAttributes *GreenTransparentVisAtt = new G4VisAttributes();
    G4VisAttributes *CyanTransparentVisAtt = new G4VisAttributes();
    G4VisAttributes *WhiteTransparentVisAtt = new G4VisAttributes();
    GreenTransparentVisAtt->SetColor(0., 1., 0., 0.2);
    CyanTransparentVisAtt->SetColor(0., 1., 1., 0.2);
    WhiteTransparentVisAtt->SetColor(1., 1., 1., 0.2);
    CyanVisAtt->SetVisibility(true);
    RedVisAtt->SetVisibility(true);
    GreenVisAtt->SetVisibility(true);
    BlueVisAtt->SetVisibility(true);
    WhiteVisAtt->SetVisibility(true);
    GreenTransparentVisAtt->SetVisibility(true);

    scintLogic->SetVisAttributes(CyanTransparentVisAtt);
    ESRFrontLogic->SetVisAttributes(RedVisAtt);
    ESRBackLogic->SetVisAttributes(RedVisAtt);
    PCBLogic->SetVisAttributes(GreenTransparentVisAtt);
    SiPMLogic->SetVisAttributes(BlueVisAtt);
    paintLogic->SetVisAttributes(WhiteTransparentVisAtt);
    return worldPhysical;
  }
  void DetectorConstruction::ConstructSDandField()
  {
  }
}