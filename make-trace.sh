#!/bin/bash

set -e

rm -rf trace-patch
git clone https://github.com/pauloasherring/trace-patch.git
cd trace-patch
patch <<"EOP"
--- patchCode.orig.py	2023-12-13 23:49:15.895056144 +0100
+++ patchCode.py	2023-12-13 23:53:27.855055994 +0100
@@ -34,8 +34,8 @@
 inprint = 'TRACE_ME_IN;\t//<<==--TracePoint!\n'
 defprint = """
 #include <sys/time.h>
-#define TRACE_ME_IN struct timeval tp ; gettimeofday ( &tp , nullptr ); printf("[%4ld.%4ld] In: %s\\n",tp.tv_sec , tp.tv_usec,__PRETTY_FUNCTION__);
-#define TRACE_ME_OUT gettimeofday ( &tp , nullptr ); printf("[%4ld.%4ld] Out: %s\\n",tp.tv_sec , tp.tv_usec,__PRETTY_FUNCTION__);
+#define TRACE_ME_IN { struct timeval tp; gettimeofday(&tp, NULL); printf("[%4ld.%4ld] In: %s\\n",tp.tv_sec , tp.tv_usec,__PRETTY_FUNCTION__); }
+#define TRACE_ME_OUT { struct timeval tp; gettimeofday(&tp , NULL); printf("[%4ld.%4ld] Out: %s\\n",tp.tv_sec , tp.tv_usec,__PRETTY_FUNCTION__); }

 """

@@ -224,7 +224,7 @@
     filelist = []
     if len(args.files ) == 0 :
         for fileext in fileExts:
-            filelist += glob.glob('.\\**\\'+fileext, recursive=args.recursive)
+            filelist += glob.glob('./'+fileext, recursive=args.recursive) + glob.glob('./**/'+fileext, recursive=args.recursive)
     else:
         # For Python 2 where argparse does not return Unicode.
         args.files = [filename.decode(sys.getfilesystemencoding())
EOP
python3 -m venv penv
./penv/bin/pip install cppclean
echo "=== Export repository"
mkdir gmenu2x
(cd .. && git ls-files | rsync -a --ignore-missing-args --files-from=- . trace-patch/gmenu2x/)
echo "=== Run patchCode"
./penv/bin/python3 patchCode.py --verbose gmenu2x/src/*.cpp
cd gmenu2x
./make-linux.sh
echo "=== Done"
