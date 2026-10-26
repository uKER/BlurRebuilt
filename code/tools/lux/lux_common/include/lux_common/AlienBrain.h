#pragma once

// alienbrain interface class
class CNxNSmartIntegrator;

class AlienBrain
{
public:
						AlienBrain();
						~AlienBrain();

	bool				GetLatest(const char *name);
	bool				CheckOut(const char *name);
	bool				CheckIn(const char *name);

	CNxNSmartIntegrator *m_pIntegrator;
};