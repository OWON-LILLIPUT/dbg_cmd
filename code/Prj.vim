"头文件处 gf打开该头文件
"添加项目之前的头文件路径 就可使用gf打开相关头文件
"添加ARM库文件路径 与安装软件路径有关
set path+=C:\Keil_v5\ARM\ARMCC\include\**
"添加项目文件路径 如何修改为相对路径更为通用???
set path+=D:\Inker\Work\Model\stm32f1_cmd_demo\code\**

" lookupfile
let g:this_project_base_dir = "."
let g:project_lookup_file   = "./filenametags"
let g:LookupFile_TagExpr    ='"./filenametags"'

" filenametags 不存在自动创建
if filereadable("filenametags") == 0
    execute "call ProjectTagUpdateLookupFile()"
endif

"  cscope.out 存在自动连接
if filereadable("cscope.out")
    execute "cs add cscope.out"
else
    execute "call Do_CsTag()"
endif 

