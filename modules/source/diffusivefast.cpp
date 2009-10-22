#include <math.h>
#include <cstdlib>
#include "diffusivefast.h"
#define expdef(x) fexp(x)
#define M_1_2PI 0.159154943 	// = 1/(2*PI)
#define M_2PI 6.283185308 		// = 2*PI

#include "iostream"
using namespace std;

/********************* HEADER ****************************/

namespace {

Space *DiffusiveFast_S;
double DiffusiveFast_ReD, DiffusiveFast_ReR;
double DiffusiveFast_NyuD, DiffusiveFast_NyuR;
double DiffusiveFast_DefaultEpsilon;
double DiffusiveFast_dfi;

// Eps for vortexes
void EpsilonV(TNode *Node, double px, double py, double &res);
void EpsilonV_faster(TNode *Node, double px, double py, double &res);
void EpsilonV_fastest(TNode *Node, double &res);
//Eps for Heat
void EpsilonH(TNode *Node, double px, double py, double &res);
void EpsilonH_faster(TNode *Node, double px, double py, double &res);
void EpsilonH_fastest(TNode *Node, double &res);

void Division_vortex(TNode *Node, double px, double py, double eps1, double &ResPX, double &ResPY, double &ResD );

} //end of namespce

/********************* SOURCE *****************************/

int InitDiffusiveFast(Space *sS, double sReD)
{
	DiffusiveFast_S = sS;
	DiffusiveFast_ReD = sReD;
	DiffusiveFast_ReR = sReD*0.5;
	DiffusiveFast_NyuD = 1/sReD;
	DiffusiveFast_NyuR = 2/sReD;
	DiffusiveFast_DefaultEpsilon = DiffusiveFast_NyuR * M_2PI;
	if (sS->BodyList) DiffusiveFast_dfi = M_2PI/sS->BodyList->size; else DiffusiveFast_dfi = 0;
	return 0;
}

// EPSILON FUNCTIONS FOR VORTEXES

namespace {
void EpsilonV(TNode *Node, double px, double py, double &res)
{
	double drx, dry, drabs2;
	int i, j, nnlsize; //Near nodes list size
	TNode **lNNode = (TNode**)Node->NearNodes->Elements; //link to NearNode link
	TNode *NNode;
	nnlsize = Node->NearNodes->size;
	
	res = 1E10; // = inf //DiffusiveFast_DefaultEpsilon * DiffusiveFast_DefaultEpsilon;
	for ( i=0; i<nnlsize; i++ )
	{
		NNode = *lNNode;
		if ( !NNode->VortexLList ) { lNNode++; continue; }
		
		TVortex** lVort = (TVortex**)NNode->VortexLList->Elements;
		TVortex* Vort;
		int lsize = NNode->VortexLList->size;
		for ( j=0; j<lsize; j++ )
		{
			Vort = *lVort;
			drx = px - Vort->rx;
			dry = py - Vort->ry;
			drabs2 = drx*drx + dry*dry;
			if ( (res > drabs2) && drabs2 )	res = drabs2;
			lVort++;
		}
		lNNode++;
	}
	res = sqrt(res);
	if (res < DiffusiveFast_dfi) res = DiffusiveFast_dfi;
}}

namespace {
void EpsilonV_faster(TNode *Node, double px, double py, double &res)
{
	double drx, dry, drabs2;
	int i, j, nnlsize; //Near nodes list size
	TNode **lNNode = (TNode**)Node->NearNodes->Elements; //link to NearNode link
	TNode *NNode;
	nnlsize = Node->NearNodes->size;
	
	res = 1E10; // = inf //DiffusiveFast_DefaultEpsilon * DiffusiveFast_DefaultEpsilon;
	if (nnlsize < 1)
	{
		
	}
	for ( i=0; i<nnlsize; i++ )
	{
		NNode = *lNNode;
		if ( !NNode->VortexLList ) { lNNode++; continue; }
		
		TVortex** lVort = (TVortex**)NNode->VortexLList->Elements;
		TVortex* Vort;
		int lsize = NNode->VortexLList->size;
		for ( j=0; j<lsize; j++ )
		{
			Vort = *lVort;
			drx = px - Vort->rx;
			dry = py - Vort->ry;
			drabs2 = fabs(drx) + fabs(dry);
			if ( (res > drabs2) && drabs2 )	res = drabs2;
			lVort++;
		}
		lNNode++;
	}
	if (res < DiffusiveFast_dfi) res = DiffusiveFast_dfi;
}}

namespace {
void EpsilonV_fastest(TNode *Node, double &res)
{
	double S=0;
	int i, n=0, nnlsize; //Near nodes list size
	TNode **lNNode = (TNode**)Node->NearNodes->Elements; //link to NearNode link
	TNode *NNode;
	nnlsize = Node->NearNodes->size;
	
	for ( i=0; i<nnlsize; i++ )
	{
		NNode = *lNNode;
		if ( NNode->VortexLList ) { S+= NNode->h*NNode->w; n+= NNode->VortexLList->size; }
		lNNode++;
	}
	if (n) 
		res = 2*sqrt(S/n); 
	else 
		; //count from far nodes
	if (res < DiffusiveFast_dfi) res = DiffusiveFast_dfi;
}}

// EPSILON FUNCTIONS FOR HEAT

namespace {
void EpsilonH(TNode *Node, double px, double py, double &res)
{
	double drx, dry, drabs2;
	int i, j, nnlsize; //Near nodes list size
	TNode **lNNode = (TNode**)Node->NearNodes->Elements; //link to NearNode link
	TNode *NNode;
	nnlsize = Node->NearNodes->size;
	
	res = 1E10; // = inf //DiffusiveFast_DefaultEpsilon * DiffusiveFast_DefaultEpsilon;
	for ( i=0; i<nnlsize; i++ )
	{
		NNode = *lNNode;
		if ( !NNode->HeatLList ) { lNNode++; continue; }
		
		TVortex** lVort = (TVortex**)NNode->HeatLList->Elements;
		TVortex* Vort;
		int lsize = NNode->HeatLList->size;
		for ( j=0; j<lsize; j++ )
		{
			Vort = *lVort;
			drx = px - Vort->rx;
			dry = py - Vort->ry;
			drabs2 = drx*drx + dry*dry;
			if ( (res > drabs2) && drabs2 )	res = drabs2;
			lVort++;
		}
		lNNode++;
	}
	res = sqrt(res);
}}

namespace {
void EpsilonH_faster(TNode *Node, double px, double py, double &res)
{
	double drx, dry, drabs2;
	int i, j, nnlsize; //Near nodes list size
	TNode **lNNode = (TNode**)Node->NearNodes->Elements; //link to NearNode link
	TNode *NNode;
	nnlsize = Node->NearNodes->size;
	
	res = 1E10; // = inf //DiffusiveFast_DefaultEpsilon * DiffusiveFast_DefaultEpsilon;
	for ( i=0; i<nnlsize; i++ )
	{
		NNode = *lNNode;
		if ( !NNode->HeatLList ) { lNNode++; continue; }
		
		TVortex** lVort = (TVortex**)NNode->HeatLList->Elements;
		TVortex* Vort;
		int lsize = NNode->HeatLList->size;
		for ( j=0; j<lsize; j++ )
		{
			Vort = *lVort;
			drx = px - Vort->rx;
			dry = py - Vort->ry;
			drabs2 = fabs(drx) + fabs(dry);
			if ( (res > drabs2) && drabs2 )	res = drabs2;
			lVort++;
		}
		lNNode++;
	}
	if (res < DiffusiveFast_dfi) res = DiffusiveFast_dfi;
}}

namespace {
void EpsilonH_fastest(TNode *Node, double &res)
{
	double S=0;
	int i, n=0, nnlsize; //Near nodes list size
	TNode **lNNode = (TNode**)Node->NearNodes->Elements; //link to NearNode link
	TNode *NNode;
	nnlsize = Node->NearNodes->size;
	
	for ( i=0; i<nnlsize; i++ )
	{
		NNode = *lNNode;
		if ( NNode->HeatLList ) { S+= NNode->h*NNode->w; n+= NNode->HeatLList->size; }
		lNNode++;
	}
	if (n) 
		res = 2*sqrt(S/n); 
	else 
		; //count from far nodes
	if (res < DiffusiveFast_dfi) res = DiffusiveFast_dfi;
}}

// OTHER FUNCTIONS

namespace {
void Division_vortex(TNode *Node, double px, double py, double eps1, double &ResPX, double &ResPY, double &ResD )
{
	int i, j;
	double drx, dry, drabs, dr1abs, drx2, dry2;
	double xx, dxx;
	double ResAbs2;

	ResPX = 0;
	ResPY = 0;
	ResD = 0.;

	TNode **lNNode = (TNode**)Node->NearNodes->Elements; //link to NearNode link
	TNode *NNode;
	int nnlsize = Node->NearNodes->size;
	for ( i=0 ; i<nnlsize; i++)
	{
		NNode = *lNNode;
		if ( !NNode->VortexLList ) { lNNode++; continue; }

		TVortex** lVort = (TVortex**)NNode->VortexLList->Elements;
		TVortex* Vort;
		int lsize = NNode->VortexLList->size;
		for ( j=0; j<lsize; j++ )
		{
			Vort = *lVort;
			drx = px - Vort->rx;
			dry = py - Vort->ry;
			drx2 = drx*drx;
			dry2 = dry*dry;
			if ( (drx2 > 1E-12) || (dry2 > 1E-12) )
			{
				drabs = sqrt(drx2 + dry2);
				dr1abs = 1/drabs;
				double exparg = -drabs*eps1;
				if ( exparg > -8 )
				{
					xx = Vort->g * expdef(-drabs*eps1); // look for define
					dxx = dr1abs * xx;
					ResPX += drx * dxx;
					ResPY += dry * dxx;
					ResD += xx;
				}
			}
			lVort++;
		}
		lNNode++;
	}
	//ResPX *= eps1; //it is in multiplier
	//ResPY *= eps1;
//	ResAbs2 = ResPX*ResPX + ResPY*ResPY;
//	if ( ResAbs2 > (ResD*ResD) ) ResD = sqrt(ResAbs2);
}}

int CalcVortexDiffusiveFast()
{
	if ( !DiffusiveFast_S->VortexList) return -1;

	int i, lsize;
	TVortex *Vort;
	
	double multiplier;

	TlList *BottomNodes = GetTreeBottomNodes();
	if ( !BottomNodes ) return -1;
	int bnlsize = BottomNodes->size; //Bottom nodes list size
	TNode** lBNode = (TNode**)BottomNodes->Elements; // link to bottom node link
	TNode* BNode; //bottom node link

	for ( i=0; i<bnlsize; i++ )
	{
		BNode = *lBNode;
		if ( !BNode->VortexLList ) { lBNode++; continue; }

/*		double epsilon, eps1;
		EpsilonV_fastest(BNode, epsilon);
		eps1 = 1/epsilon;
*/
		TVortex** lVort = (TVortex**)BNode->VortexLList->Elements;
		TVortex* Vort;
		int lsize = BNode->VortexLList->size;
		for ( int j=0; j<lsize; j++ )
		{
			Vort = *lVort;

			double epsilon, eps1;
			EpsilonV_faster(BNode, Vort->rx, Vort->ry, epsilon);
			//double rnd = (1. + double(rand())/RAND_MAX);
			//epsilon*= rnd;
			//EpsilonV_faster(BNode, Vort->rx, Vort->ry, epsilon);
			eps1 = 1/epsilon;

			double ResPX, ResPY, ResD;
			Division_vortex(BNode, Vort->rx, Vort->ry, eps1, ResPX, ResPY, ResD);

			if ( ( (ResD < 0) && (Vort->g > 0) ) || ( (ResD > 0) && (Vort->g < 0) ) ) { ResD = Vort->g; } else 
			if ( fabs(ResD) > 1E-7 )
			{
				multiplier = DiffusiveFast_NyuR/ResD*eps1*M_2PI;
				Vort->vx += ResPX * multiplier;
				Vort->vy += ResPY * multiplier;
			}

			if ( DiffusiveFast_S->BodyList ) // diffusion from cylinder only
			{
				double rabs, r1abs, erx, ery, exparg;
				rabs = sqrt(Vort->rx*Vort->rx + Vort->ry*Vort->ry);
				r1abs = 1/rabs;
				erx = Vort->rx * r1abs;
				ery = Vort->ry * r1abs;
				
				exparg = (1-rabs)*eps1;
				if (exparg > -8)
				{
					multiplier = 4 * DiffusiveFast_NyuR * eps1 * expdef(exparg);
					Vort->vx += multiplier * erx; 
					Vort->vy += multiplier * ery;
				}
			}

			lVort++;
		}

		lBNode++;
	}

	return 0;
}

int CalcHeatDiffusiveFast()
{
	if ( !DiffusiveFast_S->HeatList) return -1;

	return 0;
}

