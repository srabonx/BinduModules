#pragma once


namespace BINDU
{
	class BinduApp
	{
	public:
		BinduApp();

		virtual ~BinduApp() = default;

		virtual bool OnInit() = 0;
		virtual void Run();
		virtual bool OnDestroy() = 0;

		// Returns fps = frames per second, mspf = milliseconds per frame 
		// Returns true if a second has passed
		virtual bool CalculateFrameStats(int& fps, float& mspf, float totalTime);	

		// Accessor/ Mutator functions
		inline bool isPaused() { return m_appPaused; }

		

	protected:

		virtual void Update() = 0;
		virtual void Render() = 0;
		

	protected:

		bool	m_appPaused{ false };

	private:
		// Only a single instance can be created
		static BinduApp* m_appInstance;

	};

}
