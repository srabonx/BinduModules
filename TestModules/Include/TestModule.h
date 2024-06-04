#pragma once
#include <Win32Application.h>

class DemoClass : public BINDU::BinduApp
{
public:
	DemoClass();
	~DemoClass() override;

	void Update() override;
	void Render() override;
};