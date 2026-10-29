add_test([=[RaftMessageTest.RoundTripsEmptyAppendEntries]=]  D:/ANUBHAB/Documents/Distributed-KV/build/kv_tests.exe [==[--gtest_filter=RaftMessageTest.RoundTripsEmptyAppendEntries]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[RaftMessageTest.RoundTripsEmptyAppendEntries]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[D:/ANUBHAB/Documents/Distributed-KV/tests/raft/raft_message_test.cpp:5]==]
    WORKING_DIRECTORY [==[D:/ANUBHAB/Documents/Distributed-KV/build]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[RaftMessageTest.RoundTripsAppendEntriesWithCommand]=]  D:/ANUBHAB/Documents/Distributed-KV/build/kv_tests.exe [==[--gtest_filter=RaftMessageTest.RoundTripsAppendEntriesWithCommand]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[RaftMessageTest.RoundTripsAppendEntriesWithCommand]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[D:/ANUBHAB/Documents/Distributed-KV/tests/raft/raft_message_test.cpp:29]==]
    WORKING_DIRECTORY [==[D:/ANUBHAB/Documents/Distributed-KV/build]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[FrameTest.RoundTripsPayload]=]  D:/ANUBHAB/Documents/Distributed-KV/build/kv_tests.exe [==[--gtest_filter=FrameTest.RoundTripsPayload]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[FrameTest.RoundTripsPayload]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[D:/ANUBHAB/Documents/Distributed-KV/tests/network/frame_test.cpp:5]==]
    WORKING_DIRECTORY [==[D:/ANUBHAB/Documents/Distributed-KV/build]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[FrameTest.RejectsIncompleteHeader]=]  D:/ANUBHAB/Documents/Distributed-KV/build/kv_tests.exe [==[--gtest_filter=FrameTest.RejectsIncompleteHeader]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[FrameTest.RejectsIncompleteHeader]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[D:/ANUBHAB/Documents/Distributed-KV/tests/network/frame_test.cpp:17]==]
    WORKING_DIRECTORY [==[D:/ANUBHAB/Documents/Distributed-KV/build]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[FrameTest.RejectsIncompletePayload]=]  D:/ANUBHAB/Documents/Distributed-KV/build/kv_tests.exe [==[--gtest_filter=FrameTest.RejectsIncompletePayload]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[FrameTest.RejectsIncompletePayload]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[D:/ANUBHAB/Documents/Distributed-KV/tests/network/frame_test.cpp:28]==]
    WORKING_DIRECTORY [==[D:/ANUBHAB/Documents/Distributed-KV/build]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[MessageTest.RoundTripsRequestVote]=]  D:/ANUBHAB/Documents/Distributed-KV/build/kv_tests.exe [==[--gtest_filter=MessageTest.RoundTripsRequestVote]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[MessageTest.RoundTripsRequestVote]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[D:/ANUBHAB/Documents/Distributed-KV/tests/network/message_test.cpp:4]==]
    WORKING_DIRECTORY [==[D:/ANUBHAB/Documents/Distributed-KV/build]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[WALTest.RecoversSingleSet]=]  D:/ANUBHAB/Documents/Distributed-KV/build/kv_tests.exe [==[--gtest_filter=WALTest.RecoversSingleSet]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[WALTest.RecoversSingleSet]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[D:/ANUBHAB/Documents/Distributed-KV/tests/storage/wal_test.cpp:23]==]
    WORKING_DIRECTORY [==[D:/ANUBHAB/Documents/Distributed-KV/build]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[WALTest.RecoversMultipleCommands]=]  D:/ANUBHAB/Documents/Distributed-KV/build/kv_tests.exe [==[--gtest_filter=WALTest.RecoversMultipleCommands]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[WALTest.RecoversMultipleCommands]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[D:/ANUBHAB/Documents/Distributed-KV/tests/storage/wal_test.cpp:41]==]
    WORKING_DIRECTORY [==[D:/ANUBHAB/Documents/Distributed-KV/build]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[WALTest.EmptyWalRecoversNothing]=]  D:/ANUBHAB/Documents/Distributed-KV/build/kv_tests.exe [==[--gtest_filter=WALTest.EmptyWalRecoversNothing]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[WALTest.EmptyWalRecoversNothing]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[D:/ANUBHAB/Documents/Distributed-KV/tests/storage/wal_test.cpp:63]==]
    WORKING_DIRECTORY [==[D:/ANUBHAB/Documents/Distributed-KV/build]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[WALTest.IgnoresIncompleteFinalRecord]=]  D:/ANUBHAB/Documents/Distributed-KV/build/kv_tests.exe [==[--gtest_filter=WALTest.IgnoresIncompleteFinalRecord]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[WALTest.IgnoresIncompleteFinalRecord]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[D:/ANUBHAB/Documents/Distributed-KV/tests/storage/wal_test.cpp:70]==]
    WORKING_DIRECTORY [==[D:/ANUBHAB/Documents/Distributed-KV/build]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[WALTest.DetectsCorruptionInMiddle]=]  D:/ANUBHAB/Documents/Distributed-KV/build/kv_tests.exe [==[--gtest_filter=WALTest.DetectsCorruptionInMiddle]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[WALTest.DetectsCorruptionInMiddle]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[D:/ANUBHAB/Documents/Distributed-KV/tests/storage/wal_test.cpp:98]==]
    WORKING_DIRECTORY [==[D:/ANUBHAB/Documents/Distributed-KV/build]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[KVStoreTest.SetAndGet]=]  D:/ANUBHAB/Documents/Distributed-KV/build/kv_tests.exe [==[--gtest_filter=KVStoreTest.SetAndGet]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[KVStoreTest.SetAndGet]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[D:/ANUBHAB/Documents/Distributed-KV/tests/kv/store_test.cpp:22]==]
    WORKING_DIRECTORY [==[D:/ANUBHAB/Documents/Distributed-KV/build]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[KVStoreTest.MissingKeyReturnsNothing]=]  D:/ANUBHAB/Documents/Distributed-KV/build/kv_tests.exe [==[--gtest_filter=KVStoreTest.MissingKeyReturnsNothing]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[KVStoreTest.MissingKeyReturnsNothing]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[D:/ANUBHAB/Documents/Distributed-KV/tests/kv/store_test.cpp:34]==]
    WORKING_DIRECTORY [==[D:/ANUBHAB/Documents/Distributed-KV/build]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[KVStoreTest.DeleteRemovesKey]=]  D:/ANUBHAB/Documents/Distributed-KV/build/kv_tests.exe [==[--gtest_filter=KVStoreTest.DeleteRemovesKey]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[KVStoreTest.DeleteRemovesKey]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[D:/ANUBHAB/Documents/Distributed-KV/tests/kv/store_test.cpp:41]==]
    WORKING_DIRECTORY [==[D:/ANUBHAB/Documents/Distributed-KV/build]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[KVStoreTest.DeletingMissingKeyReturnsFalse]=]  D:/ANUBHAB/Documents/Distributed-KV/build/kv_tests.exe [==[--gtest_filter=KVStoreTest.DeletingMissingKeyReturnsFalse]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[KVStoreTest.DeletingMissingKeyReturnsFalse]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[D:/ANUBHAB/Documents/Distributed-KV/tests/kv/store_test.cpp:51]==]
    WORKING_DIRECTORY [==[D:/ANUBHAB/Documents/Distributed-KV/build]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[KVStoreTest.SetOverwritesExistingValue]=]  D:/ANUBHAB/Documents/Distributed-KV/build/kv_tests.exe [==[--gtest_filter=KVStoreTest.SetOverwritesExistingValue]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[KVStoreTest.SetOverwritesExistingValue]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[D:/ANUBHAB/Documents/Distributed-KV/tests/kv/store_test.cpp:58]==]
    WORKING_DIRECTORY [==[D:/ANUBHAB/Documents/Distributed-KV/build]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[KVStoreTest.RecoversAfterRestart]=]  D:/ANUBHAB/Documents/Distributed-KV/build/kv_tests.exe [==[--gtest_filter=KVStoreTest.RecoversAfterRestart]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[KVStoreTest.RecoversAfterRestart]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[D:/ANUBHAB/Documents/Distributed-KV/tests/kv/store_test.cpp:71]==]
    WORKING_DIRECTORY [==[D:/ANUBHAB/Documents/Distributed-KV/build]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[KVStoreTest.RecoversDeleteAfterRestart]=]  D:/ANUBHAB/Documents/Distributed-KV/build/kv_tests.exe [==[--gtest_filter=KVStoreTest.RecoversDeleteAfterRestart]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[KVStoreTest.RecoversDeleteAfterRestart]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[D:/ANUBHAB/Documents/Distributed-KV/tests/kv/store_test.cpp:98]==]
    WORKING_DIRECTORY [==[D:/ANUBHAB/Documents/Distributed-KV/build]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
set(kv_tests_TESTS [==[RaftMessageTest.RoundTripsEmptyAppendEntries]==] [==[RaftMessageTest.RoundTripsAppendEntriesWithCommand]==] [==[FrameTest.RoundTripsPayload]==] [==[FrameTest.RejectsIncompleteHeader]==] [==[FrameTest.RejectsIncompletePayload]==] [==[MessageTest.RoundTripsRequestVote]==] [==[WALTest.RecoversSingleSet]==] [==[WALTest.RecoversMultipleCommands]==] [==[WALTest.EmptyWalRecoversNothing]==] [==[WALTest.IgnoresIncompleteFinalRecord]==] [==[WALTest.DetectsCorruptionInMiddle]==] [==[KVStoreTest.SetAndGet]==] [==[KVStoreTest.MissingKeyReturnsNothing]==] [==[KVStoreTest.DeleteRemovesKey]==] [==[KVStoreTest.DeletingMissingKeyReturnsFalse]==] [==[KVStoreTest.SetOverwritesExistingValue]==] [==[KVStoreTest.RecoversAfterRestart]==] [==[KVStoreTest.RecoversDeleteAfterRestart]==])
