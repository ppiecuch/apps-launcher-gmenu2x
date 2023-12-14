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

@@ -146,7 +146,7 @@
         else:
             print (f'Warning: could not find backup file for {filename}')

-def do_patch(filelist):
+def do_patch(filelist, exclude = []):
     global args
     for filename in filelist:
         if args.verbose:
@@ -167,12 +167,13 @@
                     if len(item.body) > 2 :
                         revbody = reversed(item.body)
                         if item.return_type is not None:
-                            # Corner case: void functions
-                            if 'void' == item.return_type.name:
-                                if 'return' != item.body[-2].name:
-                                    # Function does not end with return clause
-                                    spaces = calc_insert_point(source, item.body, len(item.body)-1)
-                                    source = insert(source, '\n'+ ' '*spaces + outprint  , item.body[-1].end)
+                            if not item.name in exclude:
+                                # Corner case: void functions
+                                if 'void' == item.return_type.name:
+                                    if 'return' != item.body[-2].name:
+                                        # Function does not end with return clause
+                                        spaces = calc_insert_point(source, item.body, len(item.body)-1)
+                                        source = insert(source, '\n'+ ' '*spaces + outprint  , item.body[-1].end)
                         if isinstance( item, ast.Method):
                             # Only classes can have c/d'tor and the the in_class member
                             # Corner case: c/d'tor
@@ -220,6 +221,7 @@
     parser.add_argument('--recursive', action='store_true', default=False, help='Iteratively patch all *.c *.cpp and *.cc files within the current folder')
     parser.add_argument('--verbose', action='store_true', help='print verbose messages')
     parser.add_argument('--quiet', '-q', action='store_true', help='ignore parse errors')
+    parser.add_argument('--exclude', '-x', action='store', type=str, help='exclude names from patching')
     args = parser.parse_args()
     filelist = []
     if len(args.files ) == 0 :
@@ -243,7 +245,10 @@
         do_unpatch(filelist)
     else:
         do_backup(filelist)
-        do_patch(filelist)
+        exclude = []
+        if args.exclude:
+            exclude = args.exclude.split(',')
+        do_patch(filelist, exclude)
     status = 0

 try:

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
./penv/bin/python3 patchCode.py --verbose --exclude=mult8x4 gmenu2x/src/*.cpp
cd gmenu2x
./make-linux.sh
echo "=== Done"
