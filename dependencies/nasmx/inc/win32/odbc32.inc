%ifndef __odbc32_INC__
%define __odbc32_INC__

IMPORT CloseODBCPerfData, 0
IMPORT CollectODBCPerfData, 16
IMPORT CursorLibLockDbc, 8
IMPORT CursorLibLockDesc, 8
IMPORT CursorLibLockStmt, 8
IMPORT CursorLibTransact, 12
IMPORT DllBidEntryPoint, 36
IMPORT GetODBCSharedData, 0
IMPORT LockHandle, 12
IMPORT MpHeapAlloc
IMPORT MpHeapCompact
IMPORT MpHeapCreate
IMPORT MpHeapDestroy
IMPORT MpHeapFree
IMPORT MpHeapReAlloc
IMPORT MpHeapSize
IMPORT MpHeapValidate
IMPORT ODBCGetTryWaitValue, 0
IMPORT ODBCInternalConnectW, 36
IMPORT ODBCQualifyFileDSNW, 4
IMPORT ODBCSetTryWaitValue, 4
IMPORT OpenODBCPerfData, 4
IMPORT PostComponentError, 4
IMPORT PostODBCComponentError, 4
IMPORT PostODBCError, 16
IMPORT SQLAllocConnect, 8
IMPORT SQLAllocEnv, 4
IMPORT SQLAllocHandle, 12
IMPORT SQLAllocHandleStd, 12
IMPORT SQLAllocStmt, 8
IMPORT SQLBindCol, 24
IMPORT SQLBindParam, 32
IMPORT SQLBindParameter, 40
IMPORT SQLBrowseConnectA, 24
IMPORT SQLBrowseConnectW, 24
IMPORT SQLBulkOperations, 8
IMPORT SQLCancel, 4
IMPORT SQLCloseCursor, 4
IMPORT SQLColAttributeA, 28
IMPORT SQLColAttributeW, 28
IMPORT SQLColAttributesA, 28
IMPORT SQLColAttributesW, 28
IMPORT SQLColumnPrivilegesA, 36
IMPORT SQLColumnPrivilegesW, 36
IMPORT SQLColumnsA, 36
IMPORT SQLColumnsW, 36
IMPORT SQLConnectA, 28
IMPORT SQLConnectW, 28
IMPORT SQLCopyDesc, 8
IMPORT SQLDataSourcesA, 32
IMPORT SQLDataSourcesW, 32
IMPORT SQLDescribeColA, 36
IMPORT SQLDescribeColW, 36
IMPORT SQLDescribeParam, 24
IMPORT SQLDisconnect, 4
IMPORT SQLDriverConnectA, 32
IMPORT SQLDriverConnectW, 32
IMPORT SQLDriversA, 32
IMPORT SQLDriversW, 32
IMPORT SQLEndTran, 12
IMPORT SQLErrorA, 32
IMPORT SQLErrorW, 32
IMPORT SQLExecDirectA, 12
IMPORT SQLExecDirectW, 12
IMPORT SQLExecute, 4
IMPORT SQLExtendedFetch, 20
IMPORT SQLFetch, 4
IMPORT SQLFetchScroll, 12
IMPORT SQLForeignKeysA, 52
IMPORT SQLForeignKeysW, 52
IMPORT SQLFreeConnect, 4
IMPORT SQLFreeEnv, 4
IMPORT SQLFreeHandle, 8
IMPORT SQLFreeStmt, 8
IMPORT SQLGetConnectAttrA, 20
IMPORT SQLGetConnectAttrW, 20
IMPORT SQLGetConnectOptionA, 12
IMPORT SQLGetConnectOptionW, 12
IMPORT SQLGetCursorNameA, 16
IMPORT SQLGetCursorNameW, 16
IMPORT SQLGetData, 24
IMPORT SQLGetDescFieldA, 24
IMPORT SQLGetDescFieldW, 24
IMPORT SQLGetDescRecA, 44
IMPORT SQLGetDescRecW, 44
IMPORT SQLGetDiagFieldA, 28
IMPORT SQLGetDiagFieldW, 28
IMPORT SQLGetDiagRecA, 32
IMPORT SQLGetDiagRecW, 32
IMPORT SQLGetEnvAttr, 20
IMPORT SQLGetFunctions, 12
IMPORT SQLGetInfoA, 20
IMPORT SQLGetInfoW, 20
IMPORT SQLGetStmtAttrA, 20
IMPORT SQLGetStmtAttrW, 20
IMPORT SQLGetStmtOption, 12
IMPORT SQLGetTypeInfoA, 8
IMPORT SQLGetTypeInfoW, 8
IMPORT SQLMoreResults, 4
IMPORT SQLNativeSqlA, 24
IMPORT SQLNativeSqlW, 24
IMPORT SQLNumParams, 8
IMPORT SQLNumResultCols, 8
IMPORT SQLParamData, 8
IMPORT SQLParamOptions, 12
IMPORT SQLPrepareA, 12
IMPORT SQLPrepareW, 12
IMPORT SQLPrimaryKeysA, 28
IMPORT SQLPrimaryKeysW, 28
IMPORT SQLProcedureColumnsA, 36
IMPORT SQLProcedureColumnsW, 36
IMPORT SQLProceduresA, 28
IMPORT SQLProceduresW, 28
IMPORT SQLPutData, 12
IMPORT SQLRowCount, 8
IMPORT SQLSetConnectAttrA, 16
IMPORT SQLSetConnectAttrW, 16
IMPORT SQLSetConnectOptionA, 12
IMPORT SQLSetConnectOptionW, 12
IMPORT SQLSetCursorNameA, 12
IMPORT SQLSetCursorNameW, 12
IMPORT SQLSetDescFieldA, 20
IMPORT SQLSetDescFieldW, 20
IMPORT SQLSetDescRec, 40
IMPORT SQLSetEnvAttr, 16
IMPORT SQLSetParam, 32
IMPORT SQLSetPos, 16
IMPORT SQLSetScrollOptions, 16
IMPORT SQLSetStmtAttrA, 16
IMPORT SQLSetStmtAttrW, 16
IMPORT SQLSetStmtOption, 12
IMPORT SQLSpecialColumnsA, 40
IMPORT SQLSpecialColumnsW, 40
IMPORT SQLStatisticsA, 36
IMPORT SQLStatisticsW, 36
IMPORT SQLTablePrivilegesA, 28
IMPORT SQLTablePrivilegesW, 28
IMPORT SQLTablesA, 36
IMPORT SQLTablesW, 36
IMPORT SQLTransact, 12
IMPORT SearchStatusCode, 8
IMPORT VFreeErrors, 4
IMPORT VRetrieveDriverErrorsRowCol, 24
IMPORT ValidateErrorQueue, 8
IMPORT g_hHeapMalloc

%endif