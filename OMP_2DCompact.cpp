#include <omp.h>
#include <cmath>
#include <cstdlib>
#include <cstddef>
#include <cstring>
#include <iostream>
#include <fstream>
#include <assert.h>
#include <vector>
#include <chrono>

using namespace std;
using namespace std::chrono;

#include "Macros.hpp"
#include "Options.hpp"
#include "TimeStepping.hpp"
#include "Domain.hpp"
#include "BC.hpp" 

#include "AbstractCSolver.hpp"
//#include "CurvilinearCSolver.hpp"

#include "AbstractSingleBlockMesh.hpp"
#include "AlgebraicSingleBlockMesh.hpp"

#include "AbstractRK.hpp"
#include "TVDRK3.hpp"
//#include "RK4.hpp"
//#include "KenRK4.hpp"
//#include "LSLDDRK4.hpp"

#include "AbstractDerivatives.hpp"
#include "Pade6.hpp"

//#include "CurvilinearInterpolator.hpp"

int main(int argc, char *argv[]){

    int numThreads;
    #pragma omp parallel
    {
        numThreads = omp_get_num_threads();
    }


    cout << endl;
    cout << " --------------------" << endl;
    cout << "  Starting up Solver " << endl;
    cout << " --------------------" << endl;
    cout << endl;
    cout << " > Running on " << numThreads << " threads!" << endl;
    cout << endl;

    //Have the root rank parse the input file and broadcast it out...
    Options *opt = new Options;

    
    ////////////////////////////////////
    //Time Stepping info intialization//
    ////////////////////////////////////
    TimeStepping *ts = new TimeStepping(opt);


    ///////////////////////////
    //Boundary Condition Info//
    ///////////////////////////

    bool periodicBC[2];
    BC *bc = new BC(opt, periodicBC);

/*
    MPI_Offset disp;
    int TS, Nx, Ny, Nz;
    double time;
    if(opt->fromRestart || opt->onlyGridFromRestart){

	//If we need to read from a file, pull the dimensions from
	//the leading three doubles from the file...

	double cN[5];
	IF_RANK0{
	    FILE *ptr;
	    ptr = fopen(opt->filename.c_str(), "rb");
	    if(ptr == NULL){
		cout << "ERROR: Couldn't open file " << opt->filename << endl;
		MPI_Abort(MPI_COMM_WORLD, -10);
	    }else{
	        fread(cN, sizeof(double), 5, ptr);
	    }
	    fclose(ptr);
	}

	MPI_Bcast(cN, 5, MPI_DOUBLE, 0, MPI_COMM_WORLD);

	TS = (int)cN[0];
	time = cN[1];
	Nx = (int)cN[2];
	Ny = (int)cN[3];
	Nz = (int)cN[4];

	if(opt->fromRestart){
	    opt->timeStep = TS;
	    opt->time = time;
	}else{
	    opt->timeStep = 0;
	    opt->time = 0.0;
	}
	opt->Nx = Nx;
	opt->Ny = Ny;
	opt->Nz = Nz;

    }else{
	TS = 0;
	time = 0.0;
	opt->timeStep = TS;
	opt->time = time;
	Nx = opt->Nx;
	Ny = opt->Ny;
	Nz = opt->Nz;
	disp = 0;
    }
*/

    /////////////////////////
    //Initialize the Domain//
    /////////////////////////
    //For curvilinear coordinates these should all correspond to the max xi, eta, and zeta values
    double Lx = 1.0, Ly = 1.0;
    Domain *d = new Domain(opt, Lx, Ly);

    int Nx = d->Nx, Ny = d->Ny;
 
    /////////////////////////
    //Initialize the Solver//
    /////////////////////////
    AbstractCSolver *cs;
//  cs= new CurvilinearCSolver(c2d, d, bc, ts, opt);

    //Attach the mesh object to the solver...

//    cs->msh = new AlgebraicSingleBlockMesh(cs, d, mpiRank);



    ///////////////////////////////////////////
    //Initialize Execution Loop and RK Method//
    ///////////////////////////////////////////
    AbstractRK *rk;
/*
    if(opt->rkType == Options::TVDRK3){
        rk = new TVDRK3(cs);
    }else if(opt->rkType == Options::RK4){
	rk = new RK4(cs);
    }else if(opt->rkType == Options::KENRK4){
        rk = new KenRK4(cs);
    }else if(opt->rkType == Options::LSLDDRK4){
        rk = new LSLDDRK4(cs);
    }else{
	cout << "SHOULD NEVER GET HERE!" << endl;
    }
*/
    ///////////////////////////////
    //Set flow initial conditions//
    ///////////////////////////////
/*
    bool fromRestart = opt->fromRestart;
    if(!fromRestart){
        FOR_Z_YPEN{
            FOR_Y_YPEN{
                FOR_X_YPEN{

                    int ip = GETMAJIND_YPEN;

                    cs->rho0[ip] = 1.0;
                    cs->p0[ip]   = 11.11111111/cs->ig->gamma;
                    cs->U0[ip]   = u_temp[ip];
                    cs->V0[ip]   = v_temp[ip];
                    cs->W0[ip]   = w_temp[ip];
                }
            }
        }
    }else{

	string filename = opt->filename;

	MPI_File fh;
	MPI_Offset disp, filesize;

	MPI_File_open(MPI_COMM_WORLD, filename.c_str(), MPI_MODE_RDONLY, MPI_INFO_NULL, &fh);

	//Need to displace by the first 3 doubles and then three double fields
	disp = 5*sizeof(double)+ sizeof(double)*3.0*opt->Nx*opt->Ny*opt->Nz;

	IF_RANK0{
	    cout << " " << endl;
	}

	double *rho_in,
	       *rhoU_in,
	       *rhoV_in,
	       *rhoW_in,
	       *rhoE_in;

	cs->c2d->allocY(rho_in);
	cs->c2d->allocY(rhoU_in);
	cs->c2d->allocY(rhoV_in);
	cs->c2d->allocY(rhoW_in);
	cs->c2d->allocY(rhoE_in);

	cs->c2d->readVar(fh, disp, 1, rho_in);
	cs->c2d->readVar(fh, disp, 1, rhoU_in);
	cs->c2d->readVar(fh, disp, 1, rhoV_in);
	cs->c2d->readVar(fh, disp, 1, rhoW_in);
	cs->c2d->readVar(fh, disp, 1, rhoE_in);

	MPI_File_close(&fh);

        FOR_Z_YPEN{
            FOR_Y_YPEN{
                FOR_X_YPEN{

                    int ip = GETMAJIND_YPEN;
		    int jp = GETIND_YPEN;

	            cs->rho0[ip] = rho_in[jp];
	            cs->U0[ip] = cs->ig->solveU(rho_in[jp], rhoU_in[jp]);
	            cs->V0[ip] = cs->ig->solveU(rho_in[jp], rhoV_in[jp]);
	            cs->W0[ip] = cs->ig->solveU(rho_in[jp], rhoW_in[jp]);
	            cs->p0[ip] = cs->ig->solvep(rho_in[jp], rhoE_in[jp], cs->U0[ip], cs->V0[ip], cs->W0[ip]);

		}
	    }
	}

	cs->c2d->deallocXYZ(rho_in);
	cs->c2d->deallocXYZ(rhoU_in);
	cs->c2d->deallocXYZ(rhoV_in);
	cs->c2d->deallocXYZ(rhoW_in);
	cs->c2d->deallocXYZ(rhoE_in);

    }
*/

//    cs->setInitialConditions();



    ///////////////////////////
    //Add images to be output//
    ///////////////////////////


    //This is probably bad programming, but we'll downcast the abstract solver class pointer to the
    //solver pointer so we can access the add image function and the solver member we want to print out

 //   CurvilinearCSolver *cs_downcast = static_cast<CurvilinearCSolver*>(cs);

//    cs_downcast->addImageOutput(new PngWriter(25, 512, 512, cs->varData[4], "DIL", 2, 0.5, PngWriter::GREYSCALE));

    ////////////////////////////////////////
    //Execute the solver timestepping loop//
    ////////////////////////////////////////
    //rk->executeSolverLoop();  

    return 0;
}

/*
void CurvilinearCSolver::initialHook(){};
void CurvilinearCSolver::fullStepTemporalHook(){};
void CurvilinearCSolver::subStepTemporalHook(){};
void CurvilinearCSolver::preStepBoundaryHook(){};
void CurvilinearCSolver::postStepBoundaryHook(){};
double CurvilinearCSolver::contRHSSource(int ip){return 0.0;};
double CurvilinearCSolver::xmomRHSSource(int ip){return 0.0;};
double CurvilinearCSolver::ymomRHSSource(int ip){return 0.0;};
double CurvilinearCSolver::zmomRHSSource(int ip){return 0.0;};
double CurvilinearCSolver::engyRHSSource(int ip){return 0.0;};
*/







