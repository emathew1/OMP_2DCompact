#include "Pade6.hpp"

void Pade6::calc1stDerivField(double *dataIn, double *dataOut){

    const int numThreads = NUMTHREADSNEST;

    if(currentDir == DIRX){

	#pragma omp parallel for schedule(dynamic) num_threads(numThreads)
	FOR_Y{
 	    double *dataInLocal, *dataOutLocal;
	    int ii = j*Nx;
	    dataInLocal  = &dataIn[ii];
	    dataOutLocal = &dataOut[ii];
	    calc1stDeriv(dataInLocal, dataOutLocal);
	}


    }else if(currentDir == DIRY){
	#pragma omp parallel for schedule(dynamic) num_threads(numThreads)
	FOR_X{
	    double *dataInLocal, *dataOutLocal;
	    int ii = i*Ny;
	    dataInLocal  = &dataIn[ii];
	    dataOutLocal = &dataOut[ii];
	    calc1stDeriv(dataInLocal, dataOutLocal);
	}
    }

}

//TPB: Transform Periodic Boundaries
void Pade6::calc1stDerivField_TPB(double *dataIn, double *dataOut, double *Nm2, double *Nm1, double *Np1, double *Np2){

    const int numThreads = NUMTHREADSNEST;

    if(currentDir == DIRX){

	#pragma omp parallel for schedule(dynamic) num_threads(numThreads)
        FOR_Y{
	    int ii = j*Nx;

	    int iii = j;
	    Nm2val = Nm2[iii];
	    Nm1val = Nm1[iii];
	    Np1val = Np1[iii];
	    Np2val = Np2[iii];

	    double *dataInLocal, *dataOutLocal;
	    dataInLocal  = &dataIn[ii];
	    dataOutLocal = &dataOut[ii];
	    calc1stDeriv_TPB(dataInLocal, dataOutLocal);
	}

    }else if(currentDir == DIRY){

	#pragma omp parallel for schedule(dynamic) num_threads(numThreads)
	FOR_X{
	        int ii = i*Ny;

		int iii = i;
		Nm2val = Nm2[iii];
		Nm1val = Nm1[iii];
		Np1val = Np1[iii];
		Np2val = Np2[iii];

		double *dataInLocal, *dataOutLocal;
		dataInLocal  = &dataIn[ii];
		dataOutLocal = &dataOut[ii];
		calc1stDeriv_TPB(dataInLocal, dataOutLocal);
	}
    }

}



void Pade6::calc2ndDerivField(double *dataIn, double *dataOut){

    const int numThreads = NUMTHREADSNEST;

    if(currentDir == DIRX){

	#pragma omp parallel for schedule(dynamic) num_threads(numThreads)
	FOR_Y{
	    double *dataInLocal, *dataOutLocal;
	    int ii = j*Nx;
	    dataInLocal  = &dataIn[ii];
	    dataOutLocal = &dataOut[ii];
	    calc2ndDeriv(dataInLocal, dataOutLocal);
        }

    }else if(currentDir == DIRY){

	#pragma omp parallel for schedule(dynamic) num_threads(numThreads)
	FOR_X{
	    double *dataInLocal, *dataOutLocal;
	    int ii = i*Ny;
	    dataInLocal  = &dataIn[ii];
	    dataOutLocal = &dataOut[ii];
	    calc2ndDeriv(dataInLocal, dataOutLocal);
	}
    }

}

void Pade6::calc1stDeriv(double *phi, double *dphi){

    if(bcType == Options::PERIODIC_SOLVE){
	Compact1stPeriodic(phi, dphi);	
    }else if(bcType == Options::DIRICHLET_SOLVE){
	Compact1stDirichlet(phi, dphi);	
    }

}

void Pade6::calc1stDeriv_TPB(double *phi, double *dphi){
    if(bcType == Options::PERIODIC_SOLVE){
	Compact1stPeriodic_TPB(phi, dphi);	
    }else if(bcType == Options::DIRICHLET_SOLVE){
	cout << "CANT USE calc1stDeriv_TPB for non-periodic boundary conditions!!" << endl;
    }
}



void Pade6::calc2ndDeriv(double *phi, double *dphi){

    if(bcType == Options::PERIODIC_SOLVE){
	Compact2ndPeriodic(phi, dphi);	
    }else if(bcType == Options::DIRICHLET_SOLVE){
	Compact2ndDirichlet(phi, dphi);	
    }

}

void Pade6::multRHS1stDerivPeriodic(double dh, double *phi, int N, double *RHSvec){

    double cc2 = -b_1D/4.0;
    double cc3 = -a_1D/2.0;
    double cc4 =  a_1D/2.0;
    double cc5 =  b_1D/4.0;

    RHSvec[0] = cc2*phi[N-2] + cc3*phi[N-1] + \
                        cc4*phi[1] + cc5*phi[2];
    RHSvec[1] = cc2*phi[N-1] + cc3*phi[0] + \
                        cc4*phi[2] + cc5*phi[3];
    RHSvec[2] = cc2*phi[0] + cc3*phi[1] + \
                        cc4*phi[3] + cc5*phi[4];

    for(int ip = 3; ip < N-3; ip++){
        RHSvec[ip] = cc2*phi[ip-2] + cc3*phi[ip-1] + \
                        cc4*phi[ip+1] + cc5*phi[ip+2];

    }

    RHSvec[N-3] = cc2*phi[N-5] + cc3*phi[N-4] + \
                        cc4*phi[N-2] + cc5*phi[N-1];
    RHSvec[N-2] = cc2*phi[N-4] + cc3*phi[N-3] + \
                        cc4*phi[N-1] + cc5*phi[0];
    RHSvec[N-1] = cc2*phi[N-3] + cc3*phi[N-2] + \
                        cc4*phi[0] + cc5*phi[1];

    for(int ip = 0; ip < N; ip++){
        RHSvec[ip] /= dh;
    }

}

void Pade6::multRHS1stDerivPeriodic_TPB(double dh, double *phi, int N, double *RHSvec){

    double cc2 = -b_1D/4.0;
    double cc3 = -a_1D/2.0;
    double cc4 =  a_1D/2.0;
    double cc5 =  b_1D/4.0;

    RHSvec[0] = cc2*Nm2val + cc3*Nm1val + \
                        cc4*phi[1] + cc5*phi[2];
    RHSvec[1] = cc2*Nm1val + cc3*phi[0] + \
                        cc4*phi[2] + cc5*phi[3];
    RHSvec[2] = cc2*phi[0] + cc3*phi[1] + \
                        cc4*phi[3] + cc5*phi[4];

    for(int ip = 3; ip < N-3; ip++){
        RHSvec[ip] = cc2*phi[ip-2] + cc3*phi[ip-1] + \
                        cc4*phi[ip+1] + cc5*phi[ip+2];

    }

    RHSvec[N-3] = cc2*phi[N-5] + cc3*phi[N-4] + \
                        cc4*phi[N-2] + cc5*phi[N-1];
    RHSvec[N-2] = cc2*phi[N-4] + cc3*phi[N-3] + \
                        cc4*phi[N-1] + cc5*Np1val;
    RHSvec[N-1] = cc2*phi[N-3] + cc3*phi[N-2] + \
                        cc4*Np1val + cc5*Np2val;

    for(int ip = 0; ip < N; ip++){
        RHSvec[ip] /= dh;
    }

}


void Pade6::multRHS2ndDerivPeriodic(double dh, double *phi, int N, double *RHSvec){

    double cc1 =  b_2D/4.0;
    double cc2 =  a_2D;
    double cc3 =  -(b_2D/2.0 + 2.0*a_2D);
    double cc4 =  a_2D;
    double cc5 =  b_2D/4.0;

    RHSvec[0] = cc1*phi[N-2] + cc2*phi[N-1] + cc3*phi[0] + \
                        cc4*phi[1] + cc5*phi[2];
    RHSvec[1] = cc1*phi[N-1] + cc2*phi[0]   + cc3*phi[1] + \
                        cc4*phi[2] + cc5*phi[3];

    for(int ip = 2; ip < N-2; ip++){
        RHSvec[ip] = cc1*phi[ip-2] + cc2*phi[ip-1] + cc3*phi[ip] + \
                        cc4*phi[ip+1] + cc5*phi[ip+2];

    }

    RHSvec[N-2] = cc1*phi[N-4] + cc2*phi[N-3] + cc3*phi[N-2] + \
                        cc4*phi[N-1] + cc5*phi[0];
    RHSvec[N-1] = cc1*phi[N-3] + cc2*phi[N-2] + cc3*phi[N-1] + \
                        cc4*phi[0] + cc5*phi[1];

    for(int ip = 0; ip < N; ip++){
        RHSvec[ip] /= dh*dh;
    }

}

void Pade6::multRHS1stDerivDirichlet(double dh, double *phi, int N, double *RHSvec){


    double cc2 = -b_1D/4.0;
    double cc3 = -a_1D/2.0;
    double cc4 =  a_1D/2.0;
    double cc5 =  b_1D/4.0;

    RHSvec[0] = a1_1D*phi[0] + b1_1D*phi[1] + c1_1D*phi[2] + \
                        d1_1D*phi[3] + e1_1D*phi[4] + f1_1D*phi[5];

    RHSvec[1] = a2_1D*phi[0] + b2_1D*phi[1] + c2_1D*phi[2] + \
                        d2_1D*phi[3] + e2_1D*phi[4];

    for(int ip = 2; ip < N-2; ip++){
        RHSvec[ip] = cc2*phi[ip-2] + cc3*phi[ip-1] + \
                        cc4*phi[ip+1] + cc5*phi[ip+2];

    }

    RHSvec[N-2] = -e2_1D*phi[N-5] - d2_1D*phi[N-4] - \
                        c2_1D*phi[N-3] - b2_1D*phi[N-2] - a2_1D*phi[N-1];
    RHSvec[N-1] = -f1_1D*phi[N-6] - e1_1D*phi[N-5] - d1_1D*phi[N-4] - \
                        c1_1D*phi[N-3] - b1_1D*phi[N-2] - a1_1D*phi[N-1];

    for(int ip = 0; ip < N; ip++){
        RHSvec[ip] /= dh;
    }

}

void Pade6::multRHS2ndDerivDirichlet(double dh, double *phi, int N, double *RHSvec){


    double cc1 =  b_2D/4.0;
    double cc2 =  a_2D;
    double cc3 =  -(b_2D/2.0 + 2.0*a_2D);
    double cc4 =  a_2D;
    double cc5 =  b_2D/4.0;


    RHSvec[0] = a1_2D*phi[0] + b1_2D*phi[1] + c1_2D*phi[2] + \
                        d1_2D*phi[3] + e1_2D*phi[4];
    RHSvec[1] = a2_2D*phi[0] + b2_2D*phi[1] + c2_2D*phi[2] + \
			d2_2D*phi[3] + e2_2D*phi[4] + f2_2D*phi[5];

    for(int ip = 2; ip < N-2; ip++){
        RHSvec[ip] = cc1*phi[ip-2] + cc2*phi[ip-1] + cc3*phi[ip] +\
                        cc4*phi[ip+1] + cc5*phi[ip+2];

    }


    RHSvec[N-2] = f2_2D*phi[N-6] + e2_2D*phi[N-5] + d2_2D*phi[N-4] + \
			c2_2D*phi[N-3] + b2_2D*phi[N-2] + a2_2D*phi[N-1];
    RHSvec[N-1] = e1_2D*phi[N-5] + d1_2D*phi[N-4] + \
                        c1_2D*phi[N-3] + b1_2D*phi[N-2] + a1_2D*phi[N-1];


    for(int ip = 0; ip < N; ip++){
        RHSvec[ip] /= dh*dh;
    }


}


void Pade6::Compact1stPeriodic(double *phi, double *dphidy){

    double RHSvec[N];
    multRHS1stDerivPeriodic(dd, phi, N, RHSvec);
    cyclic(offlower_1D, diag_1D, offupper_1D, alpha_1D, alpha_1D, RHSvec, N, dphidy);

}

void Pade6::Compact1stPeriodic_TPB(double *phi, double *dphidy){

    double RHSvec[N];
    multRHS1stDerivPeriodic_TPB(dd, phi, N, RHSvec);
    cyclic(offlower_1D, diag_1D, offupper_1D, alpha_1D, alpha_1D, RHSvec, N, dphidy);

}

void Pade6::Compact2ndPeriodic(double *phi, double *dphidy){

    double RHSvec[N];
    multRHS2ndDerivPeriodic(dd, phi, N, RHSvec);
    cyclic(offlower_2D, diag_2D, offupper_2D, alpha_2D, alpha_2D, RHSvec, N, dphidy);

}


void Pade6::Compact1stDirichlet(double *phi, double *dphidy){

    double RHSvec[N];
    double work[N];

    multRHS1stDerivDirichlet(dd, phi, N, RHSvec);
    solveTri(offlower_1D, diag_1D, offupper_1D, RHSvec, dphidy, work, N);
}


void Pade6::Compact2ndDirichlet(double *phi, double *dphidy){

    double RHSvec[N];
    double work[N];

    multRHS2ndDerivDirichlet(dd, phi, N, RHSvec);
    solveTri(offlower_2D, diag_2D, offupper_2D, RHSvec, dphidy, work, N);
}

double Pade6::calcNeumann(double *f){
    return (f[0]*360.0 - f[1]*450.0 + f[2]*400.0 - f[3]*225.0 + f[4]*72.0 - f[5]*10.0)/147.0;   
}
