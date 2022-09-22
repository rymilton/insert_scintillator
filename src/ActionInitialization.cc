#include "ActionInitialization.hh"
#include "PrimaryGeneratorAction.hh"
namespace InsertScintillator
{

ActionInitialization::ActionInitialization()
{}

ActionInitialization::~ActionInitialization()
{}

void ActionInitialization::BuildForMaster() const
{
}

void ActionInitialization::Build() const
{
    SetUserAction(new PrimaryGeneratorAction);
}

}
