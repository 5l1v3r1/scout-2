//
// The main program for the heat transfer program. 
//
#include <iostream>
using namespace std;

#include "mesh.sch"

// The following two functions do the dirty work for us.
// Note that all mesh parameters are pass-by-reference
// in Scout. 
extern void initialize_mesh(UniMesh2D&);
extern void heat_transfer(UniMesh2D&);

// ----- main
//
int main(int argc, char *argv[])
{
  UniMesh heat_mesh[256, 256]; // Two-dimensional instance of the uniform mesh.
  
  initialize_mesh(heat_mesh);
  heat_transfer(heat_mesh);
  
  return 0;
}
