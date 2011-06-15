#ifndef SPACE_H_
#define SPACE_H_

#include "body.h"
#include "elementary.h"

using namespace std;

class Space
{
	public:
		Space(bool CreateVortexes,
				bool CreateHeat, 
				TVec (*sInfSpeed)(double time) = NULL);

		vector<TBody*> *BodyList;
		vector<TObj> *VortexList;
		vector<TObj> *HeatList;


		inline void StartStep(); //update local InfSpeed variables, 
		inline void FinishStep(); //update time and coord variables

		TVec InfSpeed() { return InfSpeed_link?InfSpeed_link(Time):TVec(0,0); }
		double Time, dt;

		/***************** SAVE/LOAD ******************/
		int LoadVorticityFromFile(const char* filename);
		int LoadHeatFromStupidFile(const char* filename, double g);
		int LoadHeatFromFile(const char* filename);

		int LoadBody(const char* filename);

		int PrintBody(std::ostream& os);
		int PrintVorticity(std::ostream& os);
		int PrintHeat(std::ostream& os);

		int PrintBody(const char* format);
		int PrintVorticity(const char* format);
		int PrintHeat(const char* format);

		int Save(const char *filename);
		int Load(const char *filename);

		double integral();
		double gsum();
		double gmax();
		TVec HydroDynamicMomentum();

	private:
		int Print(vector<TObj> *list, std::ostream& os);
		int Print(vector<TObj> *list, const char* format, ios::openmode mode = ios::out); //format is used for sprintf(filename, "format", time)

		TVec (*InfSpeed_link)(double time);
};

inline
void Space::StartStep()
{
	//InfSpeedXVar = InfSpeedX ? InfSpeedX(Time) : 0;
	//InfSpeedYVar = InfSpeedY ? InfSpeedY(Time) : 0;
	//Body->RotationVVar = Body->RotationV ? Body->RotationV(Time) : 0;
}

inline
void Space::FinishStep()
{
	const_for(BodyList, llbody)
	{
		TBody &body = **llbody;
		body.Rotate(body.RotSpeed(Time) * dt);
		body.Position -= InfSpeed() * dt;
	}
	Time+= dt;
}

#endif /*SPACE_H_*/