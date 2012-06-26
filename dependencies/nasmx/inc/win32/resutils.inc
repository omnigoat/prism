%ifndef __resutils_INC__
%define __resutils_INC__

IMPORT ClusWorkerCheckTerminate, 4
IMPORT ClusWorkerCreate, 12
IMPORT ClusWorkerStart, 4
IMPORT ClusWorkerTerminate, 4
IMPORT ResUtilAddUnknownProperties, 24
IMPORT ResUtilCreateDirectoryTree, 4
IMPORT ResUtilDupParameterBlock, 12
IMPORT ResUtilDupString, 4
IMPORT ResUtilEnumPrivateProperties, 20
IMPORT ResUtilEnumProperties, 20
IMPORT ResUtilEnumResources, 16
IMPORT ResUtilEnumResourcesEx, 20
IMPORT ResUtilExpandEnvironmentStrings, 4
IMPORT ResUtilFindBinaryProperty, 20
IMPORT ResUtilFindDependentDiskResourceDriveLetter, 16
IMPORT ResUtilFindDwordProperty, 16
IMPORT ResUtilFindExpandSzProperty, 16
IMPORT ResUtilFindExpandedSzProperty, 16
IMPORT ResUtilFindLongProperty, 16
IMPORT ResUtilFindMultiSzProperty, 20
IMPORT ResUtilFindSzProperty, 16
IMPORT ResUtilFreeEnvironment, 4
IMPORT ResUtilFreeParameterBlock, 12
IMPORT ResUtilGetAllProperties, 24
IMPORT ResUtilGetBinaryProperty, 28
IMPORT ResUtilGetBinaryValue, 16
IMPORT ResUtilGetCoreClusterResources, 16
IMPORT ResUtilGetDwordProperty, 28
IMPORT ResUtilGetDwordValue, 16
IMPORT ResUtilGetEnvironmentWithNetName, 4
IMPORT ResUtilGetMultiSzProperty, 28
IMPORT ResUtilGetPrivateProperties, 20
IMPORT ResUtilGetProperties, 24
IMPORT ResUtilGetPropertiesToParameterBlock, 20
IMPORT ResUtilGetProperty, 16
IMPORT ResUtilGetPropertyFormats, 20
IMPORT ResUtilGetPropertySize, 16
IMPORT ResUtilGetResourceDependency, 8
IMPORT ResUtilGetResourceDependencyByClass, 16
IMPORT ResUtilGetResourceDependencyByName, 16
IMPORT ResUtilGetResourceDependentIPAddressProps, 28
IMPORT ResUtilGetResourceName, 12
IMPORT ResUtilGetResourceNameDependency, 8
IMPORT ResUtilGetSzProperty, 20
IMPORT ResUtilGetSzValue, 8
IMPORT ResUtilIsPathValid, 4
IMPORT ResUtilIsResourceClassEqual, 8
IMPORT ResUtilPropertyListFromParameterBlock, 24
IMPORT ResUtilRemoveResourceServiceEnvironment, 12
IMPORT ResUtilResourceTypesEqual, 8
IMPORT ResUtilResourcesEqual, 8
IMPORT ResUtilSetBinaryValue, 24
IMPORT ResUtilSetDwordValue, 16
IMPORT ResUtilSetExpandSzValue, 16
IMPORT ResUtilSetMultiSzValue, 24
IMPORT ResUtilSetPrivatePropertyList, 12
IMPORT ResUtilSetPropertyParameterBlock, 28
IMPORT ResUtilSetPropertyParameterBlockEx, 32
IMPORT ResUtilSetPropertyTable, 28
IMPORT ResUtilSetPropertyTableEx, 32
IMPORT ResUtilSetResourceServiceEnvironment, 16
IMPORT ResUtilSetResourceServiceStartParameters, 20
IMPORT ResUtilSetSzValue, 16
IMPORT ResUtilSetUnknownProperties, 16
IMPORT ResUtilStartResourceService, 8
IMPORT ResUtilStopResourceService, 4
IMPORT ResUtilStopService, 4
IMPORT ResUtilTerminateServiceProcessFromResDll, 20
IMPORT ResUtilVerifyPrivatePropertyList, 8
IMPORT ResUtilVerifyPropertyTable, 24
IMPORT ResUtilVerifyResourceService, 4
IMPORT ResUtilVerifyService, 4

%endif