#pragma once


namespace BINDU
{
	class BinduApp
	{
	public:
		BinduApp();

		virtual ~BinduApp();

		virtual bool OnInit();
		virtual void Run();
		virtual bool OnDestroy();

		virtual bool CalculateFrameStats(int& fps, float& mspf, float totalTime);	// Returns fps = frames per second, mspf = milliseconds per frame

		// Accessor/ Mutator functions

		inline bool isPaused() { return m_appPaused; }

		

	protected:

		virtual void Update() = 0;
		virtual void Render() = 0;
		

	protected:

		bool	m_appPaused{ false };

	};

}
