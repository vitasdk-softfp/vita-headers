/**
 * \kernelgroup{SceModulemgr}
 * \usage{psp2kern/kernel/modulemgr.h,SceModulemgrForKernel_stub}
 */

#ifndef _PSP2KERN_KERNEL_MODULEMGR_H_
#define _PSP2KERN_KERNEL_MODULEMGR_H_

#include <vitasdk/build_utils.h>
#include <psp2common/kernel/modulemgr.h>
#include <psp2kern/types.h>
#include <psp2kern/kernel/threadmgr.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct SceKernelModuleName {
  char s[0x1C];
} SceKernelModuleName;
VITASDK_BUILD_ASSERT_EQ(0x1C, SceKernelModuleName);

typedef struct {
  SceSize size; //!< sizeof(SceKernelSegmentInfo2) (0x14)
  int perm;
  void *vaddr;
  uint32_t memsz;
  int unk_10;
} SceKernelSegmentInfo2;
VITASDK_BUILD_ASSERT_EQ(0x14, SceKernelSegmentInfo2);

typedef struct {
  SceSize size;
  SceUID modid;
  uint32_t version;
  uint32_t module_version;
  uint32_t unk10;
  void *unk14;
  uint32_t unk18;
  void *unk1C;
  void *unk20;
  char module_name[28];
  uint32_t unk40;
  uint32_t unk44;
  uint32_t nid;
  SceSize segments_num;
  union {
    struct {
      SceKernelSegmentInfo2 SegmentInfo[1];
      uint32_t addr[4];
    } seg1;
    struct {
      SceKernelSegmentInfo2 SegmentInfo[2];
      uint32_t addr[4];
    } seg2;
    struct {
      SceKernelSegmentInfo2 SegmentInfo[3];
      uint32_t addr[4];
    } seg3;
    struct {
      SceKernelSegmentInfo2 SegmentInfo[4];
      uint32_t addr[4];
    } seg4;
  };
} SceKernelModuleListInfo;
VITASDK_BUILD_ASSERT_EQ(0xB0, SceKernelModuleListInfo);

typedef struct SceKernelModuleExportEntry {
  SceNID libnid;
  void *entry; // function ptr. or vars?
} SceKernelModuleExportEntry;
VITASDK_BUILD_ASSERT_EQ(8, SceKernelModuleExportEntry);

typedef struct SceKernelModuleImportNonlinkedInfo {
	SceSize size; // 0x124
	SceUID modid;
	SceNID libnid;
	char libname[0x100];
	int data_0x10C;
	int data_0x110;
	int data_0x114;
	int data_0x118;
	int data_0x11C;
	int data_0x120;
} SceKernelModuleImportNonlinkedInfo;
VITASDK_BUILD_ASSERT_EQ(0x124, SceKernelModuleImportNonlinkedInfo);

typedef struct SceSelfAppInfo {
	int vendor_id;
	int self_type;
} SceSelfAppInfo;
VITASDK_BUILD_ASSERT_EQ(8, SceSelfAppInfo);

typedef struct SceLoadProcessParam { // size is 0x7C-bytes
	SceUInt32 sysver;
	char thread_name[0x20];
	SceUInt32 initial_thread_priority; // ex: 0x100000EC
	SceSize initial_thread_stack_size; // ex: 0x6000
	SceUInt32 unk_0x2C;
	SceUInt32 unk_0x30;
	SceKernelThreadOptParam	threadOptParam;
	int unk_0x50;
	char process_name[0x20]; // not titleid
	SceUInt32 preload_disabled;
	void *module_proc_param;
} SceLoadProcessParam;
VITASDK_BUILD_ASSERT_EQ(0x7C, SceLoadProcessParam);

typedef struct _SceKernelFunctionShimInfo {
  SceNID replaced_function_nid; /**< NID of the function that needs to be replaced */
  SceNID replacing_function_nid; /**< NID of the function that will serve as a replacement - must probably come from same library as replaced function */
} SceKernelFunctionShimInfo;
VITASDK_BUILD_ASSERT_EQ(8, SceKernelFunctionShimInfo);

typedef struct _SceKernelLibraryShimInfo {
  const char *library_name;                  /**< Name of the library the shimmed functions come from (i.e. SceThreadmgr) */
  SceUInt32 unk_04;                          /**< Always 0 ? */
  SceUInt32 function_shims_count;            /**< Size of the array pointed to by next field */
  SceKernelFunctionShimInfo* function_shims;
} SceKernelLibraryShimInfo;
VITASDK_BUILD_ASSERT_EQ(0x10, SceKernelLibraryShimInfo);

typedef struct _SceKernelCompatibilityShimInfo {
  const char *title_id;                    /**< TitleID (process name) of the app this shim applies to */
  SceUInt32 unk_04;                        /**< Always 0 ? */
  SceUInt32 library_shims_count;           /**< Size of the array pointed to by next field */
  SceKernelLibraryShimInfo *library_shims;
} SceKernelCompatibilityShimInfo;
VITASDK_BUILD_ASSERT_EQ(0x10, SceKernelCompatibilityShimInfo);


/**
 * @brief Register syscall function
 *
 * @param[in] syscall_id - register syscall id (Must be less than 0x1000)
 * @param[in] func       - syscall function
 *
 * @return none
 */
void ksceKernelRegisterSyscall(SceSize syscall_id, const void *func);

/**
 * @brief Setup kernel for modulemgr
 *
 * @note - allocate and initialize syscall table
 *
 * @return none
 */
void ksceKernelRegisterModulesAfterBoot(void);

/**
 * @brief Get module id list
 *
 * @param[in]    pid    - target pid
 * @param[in]    flags1 - unknown, use 0x7FFFFFFF
 * @param[in]    flags2 - unknown, use 1
 * @param[out]   modids - info output pointer
 * @param[inout] num    - in:list max num, out:get modid num
 *
 * @return 0 on success, < 0 on error.
 */
int ksceKernelGetModuleList(SceUID pid, int flags1, int flags2, SceUID *modids, SceSize *num);

/**
 * @par Example1: Get max to 10 kernel module info
 * @code
 * SceKernelModuleListInfo infolists[10];
 * size_t num = 10;// Get max
 * uint32_t offset = 0;
 * SceKernelModuleListInfo *info = &infolists[0];
 *
 * ksceKernelGetModuleInfoForDebugger(0x10005, infolists, &num);
 *
 * for(int i=0;i<num;i++){
 *   printf("name : %s\n", info->module_name);
 *
 *   if(info->segments_num == 1){
 *     printf("vaddr:0x%08X\n", info->seg1.SegmentInfo[0].vaddr);
 *   }else if(info->segments_num == 2){
 *     printf("vaddr:0x%08X\n", info->seg2.SegmentInfo[0].vaddr);
 *   }
 *   info = ((char *)info) + info->size;
 * }
 * @endcode
 *
 * @param[in]    pid       - target pid
 * @param[out]   infolists - infolists output
 * @param[inout] num       - Specify the maximum number of modinfolist to retrieve. If the function returns 0, it returns the number of modules loaded in the target pid in num
 *
 * @return 0 on success, < 0 on error.
 */
int ksceKernelGetModuleInfoForDebugger(SceUID pid, SceKernelModuleListInfo *infolists, SceSize *num);

/**
 * @brief Get module info
 *
 * @param[in]  pid   - target pid
 * @param[in]  modid - target module id
 * @param[out] info  - info output pointer
 *
 * @return 0 on success, < 0 on error.
 */
int ksceKernelGetModuleInfo(SceUID pid, SceUID modid, SceKernelModuleInfo *info);

/**
 * @brief Get module info mini by module address
 *
 * @param[in]  pid               - target pid
 * @param[in]  module_addr       - target module address, Also possible with data segment address
 * @param[out] module_nid        - can also set NULL, module nid output
 * @param[out] program_text_addr - can also set NULL, program text addr output
 * @param[out] module_name       - can also set NULL, module name output
 *
 * @return 0 on success, < 0 on error.
 */
int ksceKernelGetModuleInfoMinByAddr(SceUID pid, const void *module_addr, uint32_t *module_nid, const void **program_text_addr, SceKernelModuleName *module_name);

/**
 * @brief Get module info (internal)
 *
 * @param[in]  modid - target module id
 * @param[out] info  - module_info output pointer
 *
 * @return 0 on success, < 0 on error.
 */
int ksceKernelGetModuleCB(SceUID modid, void **info);

/**
 * @brief Get module info by address (internal)
 *
 * @param[in]  pid         - target process id
 * @param[in]  module_addr - target module address
 * @param[out] info        - module_info output pointer
 *
 * @return 0 on success, < 0 on error.
 */
int ksceKernelGetModuleCBByAddr(SceUID pid, const void *module_addr, void **info);

/**
 * @brief Get module id by module address
 *
 * @param[in] pid         - target pid
 * @param[in] module_addr - target module address
 *
 * @return modid on success, < 0 on error.
 */
SceUID ksceKernelGetModuleIdByAddrForDebugger(SceUID pid, const void *module_addr);

/**
 * @brief search kernel module by module name
 *
 * @param[in] module_name - target module name
 *
 * @return modid on success, < 0 on error.
 */
SceUID ksceKernelSearchModuleByName(const char *module_name);

/**
 * @brief Get fw info
 *
 * @param[out] data - fw info output pointer
 *
 * @return 0 on success, < 0 on error.
 */
int ksceKernelGetSystemSwVersion(SceKernelFwInfo *data);

/**
 * @brief load module (kernel only)
 *
 * @param[in] path   - module path
 * @param[in] flags  - unknown, set zero
 * @param[in] option - unknown
 *
 * @return modid on success, < 0 on error.
 */
SceUID ksceKernelLoadModule(const char *path, int flags, SceKernelLMOption *option);

/**
 * @brief start module (kernel only)
 *
 * @param[in]  modid  - target module id
 * @param[in]  args   - module start args
 * @param[in]  argp   - module start argp
 * @param[in]  flags  - unknown, set zero
 * @param[in]  option - unknown
 * @param[out] status - module_start res, SCE_KERNEL_START_SUCCESS etc...
 *
 * @return 0 on success, < 0 on error.
 */
int ksceKernelStartModule(SceUID modid, SceSize args, void *argp, int flags, SceKernelLMOption *option, int *status);

/**
 * @brief load and start module (kernel only)
 *
 * @param[in]  path   - module path
 * @param[in]  args   - module start args
 * @param[in]  argp   - module start argp
 * @param[in]  flags  - unknown, set zero
 * @param[in]  option - unknown
 * @param[out] status - module_start res, SCE_KERNEL_START_SUCCESS etc...
 *
 * @return modid on success, < 0 on error.
 */
SceUID ksceKernelLoadStartModule(const char *path, SceSize args, void *argp, int flags, SceKernelLMOption *option, int *status);

/**
 * @brief stop module (kernel only)
 *
 * @param[in]  modid  - target module id
 * @param[in]  args   - module stop args
 * @param[in]  argp   - module stop argp
 * @param[in]  flags  - unknown, set zero
 * @param[in]  option - unknown
 * @param[out] status - module_stop res, SCE_KERNEL_STOP_SUCCESS etc...
 *
 * @return 0 on success, < 0 on error.
 */
int ksceKernelStopModule(SceUID modid, SceSize args, void *argp, int flags, SceKernelULMOption *option, int *status);

/**
 * @brief unload module (kernel only)
 *
 * @param[in]  modid  - target module id
 * @param[in]  flags  - unknown, set zero
 * @param[in]  option - unknown
 *
 * @return 0 on success, < 0 on error.
 */
int ksceKernelUnloadModule(SceUID modid, int flags, SceKernelULMOption *option);

/**
 * @brief stop and unload module (kernel only)
 *
 * @param[in]  modid  - target module id
 * @param[in]  args   - module stop args
 * @param[in]  argp   - module stop argp
 * @param[in]  flags  - unknown, set zero
 * @param[in]  option - unknown
 * @param[out] status - module_stop res, SCE_KERNEL_STOP_SUCCESS etc...
 *
 * @return 0 on success, < 0 on error.
 */
int ksceKernelStopUnloadModule(SceUID modid, SceSize args, void *argp, int flags, SceKernelULMOption *option, int *status);

/**
 * @brief load module
 *
 * @param[in]  pid    - target pid
 * @param[in]  path   - module path
 * @param[in]  flags  - unknown, set zero
 * @param[in]  option - unknown
 *
 * @return modid on success, < 0 on error.
 */
SceUID ksceKernelLoadModuleForPid(SceUID pid, const char *path, int flags, SceKernelLMOption *option);

/**
 * @brief start module
 *
 * @param[in]  pid    - target pid
 * @param[in]  modid  - target module id
 * @param[in]  args   - module start args
 * @param[in]  argp   - module start argp
 * @param[in]  flags  - unknown, set zero
 * @param[in]  option - unknown
 * @param[out] status - module_start res, SCE_KERNEL_START_SUCCESS etc...
 *
 * @return 0 on success, < 0 on error.
 */
int ksceKernelStartModuleForPid(SceUID pid, SceUID modid, SceSize args, void *argp, int flags, SceKernelLMOption *option, int *status);

/**
 * @brief load and start module
 *
 * @param[in]  pid    - target pid
 * @param[in]  path   - module path
 * @param[in]  args   - module start args
 * @param[in]  argp   - module start argp
 * @param[in]  flags  - unknown, set zero
 * @param[in]  option - unknown
 * @param[out] status - module_start res, SCE_KERNEL_START_SUCCESS etc...
 *
 * @return modid on success, < 0 on error.
 */
SceUID ksceKernelLoadStartModuleForPid(SceUID pid, const char *path, SceSize args, void *argp, int flags, SceKernelLMOption *option, int *status);

/**
 * @brief stop module
 *
 * @param[in]  pid    - target pid
 * @param[in]  modid  - target module id
 * @param[in]  args   - module stop args
 * @param[in]  argp   - module stop argp
 * @param[in]  flags  - unknown, set zero
 * @param[in]  option - unknown
 * @param[out] status - module_stop res, SCE_KERNEL_STOP_SUCCESS etc...
 *
 * @return 0 on success, < 0 on error.
 */
int ksceKernelStopModuleForPid(SceUID pid, SceUID modid, SceSize args, void *argp, int flags, SceKernelULMOption *option, int *status);

/**
 * @brief unload module
 *
 * @param[in]  pid    - target pid
 * @param[in]  modid  - target module id
 * @param[in]  flags  - unknown, set zero
 * @param[in]  option - unknown
 *
 * @return 0 on success, < 0 on error.
 */
int ksceKernelUnloadModuleForPid(SceUID pid, SceUID modid, int flags, SceKernelULMOption *option);

/**
 * @brief stop and unload module
 *
 * @param[in]  pid    - target pid
 * @param[in]  modid  - target module id
 * @param[in]  args   - module stop args
 * @param[in]  argp   - module stop argp
 * @param[in]  flags  - unknown, set zero
 * @param[in]  option - unknown
 * @param[out] status - module_stop res, SCE_KERNEL_STOP_SUCCESS etc...
 *
 * @return 0 on success, < 0 on error.
 */
int ksceKernelStopUnloadModuleForPid(SceUID pid, SceUID modid, SceSize args, void *argp, int flags, SceKernelULMOption *option, int *status);

/**
 * @brief load and start module as shared module
 *
 * @param[in]  pid    - target pid
 * @param[in]  path   - module path
 * @param[in]  args   - module start args
 * @param[in]  argp   - module start argp
 * @param[in]  flags  - unknown, set zero
 * @param[in]  option - unknown
 * @param[out] status - module_start res, SCE_KERNEL_START_SUCCESS etc...
 *
 * @return modid on success, < 0 on error.
 */
SceUID ksceKernelLoadStartSharedModuleForPid(SceUID pid, const char *path, SceSize args, void *argp, int flags, SceKernelLMOption *option, int *status);

/**
 * @brief stop and unload module as shared module
 *
 * @param[in]  pid    - target pid
 * @param[in]  modid  - target module id
 * @param[in]  args   - module stop args
 * @param[in]  argp   - module stop argp
 * @param[in]  flags  - unknown, set zero
 * @param[in]  option - unknown
 * @param[out] status - module_stop res, SCE_KERNEL_STOP_SUCCESS etc...
 *
 * @return 0 on success, < 0 on error.
 */
int ksceKernelStopUnloadSharedModuleForPid(SceUID pid, SceUID modid, SceSize args, void *argp, int flags, SceKernelULMOption *option, int *status);

/**
 * @brief mount bootfs (load bootfs module)
 *
 * @param[in] bootImagePath - bootfs module path
 *
 * @return 0 on success, < 0 on error.
 */
int ksceKernelMountBootfs(const char *bootImagePath);

/**
 * @brief unmount bootfs (unload bootfs module)
 *
 * @return 0 on success, < 0 on error.
 */
int ksceKernelUmountBootfs(void);

/**
 * @brief Get the main module id for a given process.
 * @param pid The process to query.
 * @return the UID of the module else < 0 for an error.
 */
SceUID ksceKernelGetModuleIdByPid(SceUID pid);

/**
 * @brief Get the module path
 *
 * @param[in]  pid     - target pid
 * @param[out] path    - module path output
 * @param[in]  pathlen - path output max len
 *
 * @return 0 on success, < 0 on error.
 */
int ksceKernelGetModulePath(SceUID modid, char *path, SceSize pathlen);

/**
 * @brief Get library info
 *
 * @param[in]  pid   - target pid
 * @param[in]  modid - target library id
 * @param[out] info  - info output
 *
 * @return 0 on success, < 0 on error.
 */
int ksceKernelGetLibraryInfoForDebugger(SceUID pid, SceUID library_id, SceKernelModuleLibraryInfo *info);


void ksceKernelFinalizeKbl(void);
int ksceKernelGetExportedLibraryListInModule(SceUID pid, SceUID modid, SceUID *library_ids, SceSize *num);
int ksceKernelGetImportedLibraryListInModule(SceUID pid, SceUID modid, SceUID *library_ids, SceSize *num);
int ksceKernelGetLibEntCBListForSyslibtrace(void **ppList, SceSize *num);
int ksceKernelGetLibraryList(SceUID pid, SceUID *library_ids, SceSize *num);
int ksceKernelGetLibraryClientList(SceUID pid, SceUID library_id, SceUID *modids, SceSize *num, SceSize cpy_skip_num);
int ksceKernelGetLibraryDBFlags(SceUID pid, int *pFlags);
int ksceKernelGetLibraryExportInfoForDebugger(SceUID pid, SceUID library_id, SceKernelModuleExportEntry *list, SceSize *num, SceSize cpy_skip_num);
int ksceKernelGetLostLibraryInfo(SceUID pid, SceUID modid, SceNID libnid, SceKernelModuleImportNonlinkedInfo *info);
int ksceKernelGetLostLibraryList(SceUID pid, void *a2, SceSize *num);
int ksceKernelGetLostLibraryListInModule(SceUID pid, SceUID modid, void *pList, SceSize *num);
int ksceKernelGetMetaDataForDebugger(SceUID pid, SceUID uModuleId, void **start, void **stop);
void *ksceKernelGetModuleEntryPoint(SceUID modid);
int ksceKernelGetModuleFingerprint(SceUID moduleId, SceUInt32 *pFingerprint);
int ksceKernelGetModuleIsSharedByAddr(SceUID pid, void *addr);
int ksceKernelGetProgramIdentificationInfo(const char *path, SceUInt64 *pAuthid, SceSelfAppInfo *pInfo);
int ksceKernelGetStubInfoForDebugger(SceUID pid, SceUID stubid, void *a3);
int ksceKernelGetStubList(SceUID pid, SceUID *stub, SceSize *num);
int ksceKernelGetStubNidTableForDebugger(SceUID pid, SceUID stubid, void *a3, SceSize *num, SceSize cpy_skip_num);
int ksceKernelLoadPreloadingModules(SceUID pid, const SceLoadProcessParam *pParam, int flags);
SceUID ksceKernelLoadProcessImage(SceUID pid, const char *path, int flags, SceSelfAuthInfo *auth_info, SceLoadProcessParam *param, SceKernelCompatibilityShimInfo *shim_info);
int ksceKernelLoadPtLoadSegForFwloader(const char *path, int e_phnum, void *buffer, SceSize bufsize, int zero_unk, SceSize *bytes_read);
int ksceKernelModuleUnloadMySelf(void);
int ksceKernelStartPreloadingModules(SceUID pid);
int ksceKernelUnloadProcessModules(SceUID pid);


/* For backwards compatibility */

#define ksceKernelGetModuleInternal ksceKernelGetModuleCB
#define ksceKernelGetProcessMainModule ksceKernelGetModuleIdByPid


#ifdef __cplusplus
}
#endif

#endif /* _PSP2KERN_KERNEL_MODULEMGR_H_ */
