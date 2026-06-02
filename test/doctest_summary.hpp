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

		std::set<size_t> mSubcaseHashes;
		std::set<size_t> mSubcaseWarnHashes;
		std::set<size_t> mSubcaseCheckHashes;
		std::set<size_t> mSubcaseRequireHashes;

		bool FailedAny() const { return mWarnFailed || mCheckFailed || mRequireFailed; }
	};

	struct TotalStats
	{
		size_t mAssertsFailed = 0;
		size_t mCasesFailed = 0;
		size_t mTestsFailed = 0;

		TotalStats &operator += ( const TotalStats &inRHS )
		{
			mAssertsFailed += inRHS.mAssertsFailed;
			mCasesFailed += inRHS.mCasesFailed;
			mTestsFailed += inRHS.mTestsFailed;

			return *this;
		}
	};

	std::ostream &mOutStream;
	const doctest::ContextOptions &mOptions;
	const doctest::TestCaseData *mTestCase;
	std::mutex mMutex;

	std::vector<doctest::SubcaseSignature> mSubcaseStack;
	size_t mSubcaseLevel;

	std::unordered_map<std::string, std::unordered_map<std::string, Stats>> mStats;
	std::unordered_map<std::string, std::vector<std::string>> mTestOrder;

	SummaryReporter( const doctest::ContextOptions &inOptions ) :
		mOutStream( *inOptions.cout ),
		mOptions( inOptions )
	{}

	void report_query( const doctest::QueryData & ) override {}

	void test_run_start() override {}

	void test_run_end( const doctest::TestRunStats &inRunStats ) override
	{
		mOutStream << "\n";

		TotalStats globalWarn, globalCheck, globalRequired;

		for ( const auto &[suiteName, testCases] : mStats ) {

			TotalStats suiteWarn, suiteCheck, suiteRequired;

			bool anyFailed = false;
			for ( const auto &[testName, stats] : testCases ) {
				if ( stats.FailedAny() ) {
					anyFailed = true;

					if ( stats.mWarnFailed ) {
						suiteWarn.mAssertsFailed += stats.mWarnFailed;
						suiteWarn.mCasesFailed += stats.mSubcaseWarnHashes.size();
						suiteWarn.mTestsFailed++;
					}

					if ( stats.mCheckFailed ) {
						suiteCheck.mAssertsFailed += stats.mCheckFailed;
						suiteCheck.mCasesFailed += stats.mSubcaseCheckHashes.size();
						suiteCheck.mTestsFailed++;
					}

					if ( stats.mRequireFailed ) {
						suiteRequired.mAssertsFailed += stats.mRequireFailed;
						suiteRequired.mCasesFailed += stats.mSubcaseRequireHashes.size();
						suiteRequired.mTestsFailed++;
					}
				}
			}

			if ( !anyFailed ) continue;

			globalWarn += suiteWarn;
			globalCheck += suiteCheck;
			globalRequired += suiteRequired;

			const auto &testCaseNames = mTestOrder[suiteName];

			mOutStream << suiteName;

			if ( suiteWarn.mTestsFailed ) {
				mOutStream << doctest::Color::Yellow << "  Warn: ";
				mOutStream << suiteWarn.mTestsFailed << " tests, ";
				mOutStream << suiteWarn.mCasesFailed << " cases, ";
				mOutStream << suiteWarn.mAssertsFailed << " asserts.";
			}

			if ( suiteCheck.mTestsFailed ) {
				mOutStream << doctest::Color::Red << "  Check: ";
				mOutStream << suiteCheck.mTestsFailed << " tests, ";
				mOutStream << suiteCheck.mCasesFailed << " cases, ";
				mOutStream << suiteCheck.mAssertsFailed << " asserts.";
			}

			if ( suiteRequired.mTestsFailed ) {
				mOutStream << doctest::Color::BrightRed << "  Required: ";
				mOutStream << suiteRequired.mTestsFailed << " tests, ";
				mOutStream << suiteRequired.mCasesFailed << " cases, ";
				mOutStream << suiteRequired.mAssertsFailed << " asserts.";
			}

			mOutStream << doctest::Color::None << std::endl;


			for ( const auto &testName : testCaseNames ) {
				const auto &stats = testCases.at( testName );
				if ( stats.FailedAny() ) {
					mOutStream << "  " << testName;

					if ( stats.mWarnFailed ) {
						mOutStream << doctest::Color::Yellow;
						mOutStream << "  " << stats.mWarnFailed << " warns";
						mOutStream << " (" << stats.mSubcaseWarnHashes.size() << "/" << stats.mSubcaseHashes.size();
						mOutStream << " cases)";
					}

					if ( stats.mCheckFailed ) {
						mOutStream << doctest::Color::Red;
						mOutStream << "  " << stats.mCheckFailed << " checks";
						mOutStream << " (" << stats.mSubcaseCheckHashes.size() << "/" << stats.mSubcaseHashes.size();
						mOutStream << " cases)";
					}

					if ( stats.mRequireFailed ) {
						mOutStream << doctest::Color::BrightRed;
						mOutStream << "  " << stats.mRequireFailed << " requires";
						mOutStream << " (" << stats.mSubcaseRequireHashes.size() << "/" << stats.mSubcaseHashes.size();
						mOutStream << " cases)";
					}

					mOutStream << doctest::Color::None << std::endl;
				}
			}
		}

		mOutStream << "\n";

		mOutStream << doctest::Color::BrightWhite << "Summary:";

		if ( globalWarn.mTestsFailed ) {
			mOutStream << doctest::Color::Yellow << "  Warn: ";
			mOutStream << globalWarn.mTestsFailed << " tests, ";
			mOutStream << globalWarn.mCasesFailed << " cases, ";
			mOutStream << globalWarn.mAssertsFailed << " asserts.";
		}

		if ( globalCheck.mTestsFailed ) {
			mOutStream << doctest::Color::Red << "  Check: ";
			mOutStream << globalCheck.mTestsFailed << " tests, ";
			mOutStream << globalCheck.mCasesFailed << " cases, ";
			mOutStream << globalCheck.mAssertsFailed << " asserts.";
		}

		if ( globalRequired.mTestsFailed ) {
			mOutStream << doctest::Color::BrightRed << "  Required: ";
			mOutStream << globalRequired.mTestsFailed << " tests, ";
			mOutStream << globalRequired.mCasesFailed << " cases, ";
			mOutStream << globalRequired.mAssertsFailed << " asserts.";
		}

		mOutStream << doctest::Color::None << std::endl;
	}

	void test_case_start( const doctest::TestCaseData &inTestCase ) override
	{
		std::lock_guard<std::mutex> lock( mMutex );

		mTestCase = &inTestCase;
		mSubcaseStack.clear();
		mSubcaseLevel = 0;

		mStats[mTestCase->m_test_suite][mTestCase->m_name];
		mTestOrder[mTestCase->m_test_suite].emplace_back( mTestCase->m_name );
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

		size_t caseHash = 0;
		for ( size_t i = 0; i < mSubcaseLevel; ++i ) {
			size_t currHash = std::hash<std::string>{}( mSubcaseStack[i].m_name.c_str() );
			caseHash = currHash ^ ( caseHash << 1 );
		}

		auto &currStats = mStats[mTestCase->m_test_suite][mTestCase->m_name];

		currStats.mSubcaseHashes.insert( caseHash );

		if ( inAssert.m_failed ) {
			if ( inAssert.m_at & doctest::assertType::is_warn ) {
				currStats.mWarnFailed++;
				currStats.mSubcaseWarnHashes.insert( caseHash );
			}
			if ( inAssert.m_at & doctest::assertType::is_check ) {
				currStats.mCheckFailed++;
				currStats.mSubcaseCheckHashes.insert( caseHash );
			}
			if ( inAssert.m_at & doctest::assertType::is_require ) {
				currStats.mRequireFailed++;
				currStats.mSubcaseRequireHashes.insert( caseHash );
			}
		}
	}

	void log_message( const doctest::MessageData &inMessage ) override
	{
		std::lock_guard<std::mutex> lock( mMutex );
	}

	void test_case_skipped( const doctest::TestCaseData & ) override {}
};

REGISTER_REPORTER( "summary", 1, SummaryReporter );