#pragma once


namespace BINDU
{
	class BinduApp
	{
	public:

		virtual bool OnInit() = 0;
		virtual void Run() = 0;
		virtual bool OnDestroy() = 0;


	protected:

		virtual void Update() = 0;
		virtual void Render() = 0;

	};

}
