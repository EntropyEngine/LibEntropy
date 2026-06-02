#include "doctest.h"

#include <mutex>
#include <unordered_map>

struct SummaryReporter : public doctest::IReporter
{
	struct Stats
	{
		size_t mWarnFailed = 0;
		size_t mCheckFailed = 0;
		size_t mRequireFailed = 0;

		bool FailedAny() const { return mWarnFailed || mCheckFailed || mRequireFailed; }
	};

	std::ostream &mOutStream;
	const doctest::ContextOptions &mOptions;
	const doctest::TestCaseData *mTestCase;
	std::mutex mMutex;

	std::vector<doctest::SubcaseSignature> mSubcaseStack;
	size_t mSubcaseLevel;

	std::unordered_map<std::string, std::unordered_map<std::string, Stats>> mStats;

	SummaryReporter( const doctest::ContextOptions &inOptions ) :
		mOutStream( *inOptions.cout ),
		mOptions( inOptions )
	{}

	void report_query( const doctest::QueryData & ) override {}

	void test_run_start() override {}

	void test_run_end( const doctest::TestRunStats &inRunStats ) override
	{
		mOutStream << "\nSummary\n\n";

		for ( const auto &[suiteName, testCases] : mStats ) {

			bool anyFailed = false;
			for ( const auto &[testName, stats] : testCases ) {
				if ( stats.FailedAny() ) anyFailed = true;
			}

			if ( !anyFailed ) continue;

			mOutStream << suiteName << std::endl;
			for ( const auto &[testName, stats] : testCases ) {
				if ( stats.FailedAny() ) {
					mOutStream << "  " << testName << std::endl;

					if ( stats.mWarnFailed )
						mOutStream << doctest::Color::Yellow << "    warn failed:    " << stats.mWarnFailed << std::endl;

					if ( stats.mCheckFailed )
						mOutStream << doctest::Color::Red << "    check failed:   " << stats.mCheckFailed << std::endl;

					if ( stats.mRequireFailed )
						mOutStream << doctest::Color::BrightRed << "    require failed: " << stats.mRequireFailed << std::endl;

					mOutStream << doctest::Color::None << std::endl;
				}
			}
		}
	}

	void test_case_start( const doctest::TestCaseData &inTestCase ) override
	{
		std::lock_guard<std::mutex> lock( mMutex );

		mTestCase = &inTestCase;
		mSubcaseStack.clear();
		mSubcaseLevel = 0;

		mStats[mTestCase->m_test_suite][mTestCase->m_name];
	}

	void test_case_reenter( const doctest::TestCaseData &inTestCase ) override
	{
		std::lock_guard<std::mutex> lock( mMutex );

		mSubcaseStack.clear();
	}

	void test_case_end( const doctest::CurrentTestCaseStats  &inTestStats ) override
	{
		std::lock_guard<std::mutex> lock( mMutex );

		// Do something
	}

	void test_case_exception( const doctest::TestCaseException  &inException ) override
	{
		std::lock_guard<std::mutex> lock( mMutex );
	}

	void subcase_start( const doctest::SubcaseSignature &inSubcase ) override
	{
		std::lock_guard<std::mutex> lock( mMutex );

		mSubcaseStack.push_back( inSubcase );
		++mSubcaseLevel;
	}

	void subcase_end() override
	{
		std::lock_guard<std::mutex> lock( mMutex );

		--mSubcaseLevel;
	}

	void log_assert( const doctest::AssertData &inAssert ) override
	{
		// TODO: skip success?

		std::lock_guard<std::mutex> lock( mMutex );

		auto &currStats = mStats[mTestCase->m_test_suite][mTestCase->m_name];

		if ( inAssert.m_failed ) {
			if ( inAssert.m_at & doctest::assertType::is_warn ) currStats.mWarnFailed++;
			if ( inAssert.m_at & doctest::assertType::is_check ) currStats.mCheckFailed++;
			if ( inAssert.m_at & doctest::assertType::is_require ) currStats.mRequireFailed++;
		}
	}

	void log_message( const doctest::MessageData &inMessage ) override
	{
		std::lock_guard<std::mutex> lock( mMutex );
	}

	void test_case_skipped( const doctest::TestCaseData & ) override {}
};

REGISTER_REPORTER( "summary", 1, SummaryReporter );