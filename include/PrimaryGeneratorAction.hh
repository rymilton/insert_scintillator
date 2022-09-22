#ifndef PrimaryGeneratorAction_h
#define PrimaryGeneratorAction_h 1

#include "G4VUserPrimaryGeneratorAction.hh"
#include "G4ParticleGun.hh"
#include "G4Event.hh"

namespace InsertScintillator
{

class PrimaryGeneratorAction : public G4VUserPrimaryGeneratorAction
{
  public:
    PrimaryGeneratorAction();
    ~PrimaryGeneratorAction() override;

    void GeneratePrimaries(G4Event* ) override;

    G4ParticleGun* GetParticleGun() {return fParticleGun;}

  private:
    G4ParticleGun* fParticleGun = nullptr; // G4 particle gun
};

}

#endif