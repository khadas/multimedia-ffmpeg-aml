package ffmpeg

import (
    "android/soong/android"
    "android/soong/cc"
    "os/exec"
    "fmt"
)

func init() {
    android.RegisterModuleType("ffmpeg_defaults", ffmpegDefaultsFactory)
}

func ffmpegDefaultsFactory() (android.Module) {
    module := cc.DefaultsFactory()
    android.AddLoadHook(module, ffmpegDefaults)
    return module
}

func ffmpegDefaults(ctx android.LoadHookContext) {
    type props struct {
        Cflags []string
        //Srcs []string
    }
    p := &props{}
    p.Cflags = getCflags(ctx)
    //p.Srcs = getSrcs(ctx)
    ctx.AppendProperties(p)
}

func getCflags(ctx android.BaseContext) ([]string) {
    var cflags []string
    var arch = string(ctx.DeviceConfig().DeviceArch())
    fmt.Println("Define TARGET_ARCH:",arch)
    if arch == "arm" || arch == "arm64" {
        cflags = append(cflags,
                        "-std=c99",
                        "-llvm",
                        "-DDISABLE_NEONINTR",
                        "-D_FILE_OFFSET_BITS=64",
                        "-D_LARGEFILE_SOURCE",
                        "-D_ISOC99_SOURCE",
                        "-D_POSIX_C_SOURCE=200112")
    }

    var dir = string(ctx.Config().Getenv("PWD"))
    gcmd := "cd " + dir + "/" + ctx.ModuleDir() + "&& git log | grep commit -m 1 | cut -d' ' -f 2"
    gitout, giterr := exec.Command("/bin/bash", "-c", gcmd).CombinedOutput()
    if giterr != nil {
        fmt.Printf("get git err, gitout %s\n", gitout)
    } else {
        fmt.Printf("dir: %s, git: %s\n", dir + "/" + ctx.ModuleDir(), string(gitout))
        gitversion := "" + string(gitout)
        buildname := " (" + string(ctx.Config().Getenv("LOGNAME")) + " "
        timecmd, timeerr := exec.Command("/bin/bash", "-c", "date").CombinedOutput()
        buildtime := " " + string(timecmd) + ")"
        if timeerr != nil {
            fmt.Println("get time error")
        }
        ver := "-DGIT_INFO=" + "\"" + gitversion + buildname + buildtime + "\""
        fmt.Println(string(ver))
        cflags = append(cflags, ver)
    }
    fmt.Println(string(ctx.Config().Getenv("LOGNAME")))

    return cflags
}

//func getSrcs(ctx android.BaseContext) ([]string) {
//    var srcs []string
//    var arch = string(ctx.DeviceConfig().DeviceArch())
//    var dir = string(ctx.Config().Getenv("PWD")) + "/" + ctx.ModuleDir()
//    fmt.Printf("dir %s\n", dir)
//    if arch == "arm" {
//        file, err := os.Open(string(dir) + "/" + "ffmpeg.arm.src")
//        if err != nil {
//            fmt.Println(err)
//            return srcs
//        }
//        defer file.Close()
//        rd := bufio.NewReader(file)
//        for {
//            src, _, err := rd.ReadLine()
//            if err != nil || err == io.EOF {
//                break
//            }
//            srcs = append(srcs, string(src))
//        }
//    } else if arch == "arm64" {
//        file, err := os.Open(string(dir) + "/" + "ffmpeg.arm64.src")
//        if err != nil {
//            fmt.Println(err)
//            return srcs
//        }
//        defer file.Close()
//        rd := bufio.NewReader(file)
//        for {
//            src, _, err := rd.ReadLine()
//            if err != nil || err == io.EOF {
//                break
//            }
//            srcs = append(srcs, string(src))
//        }
//    }
//    return srcs;
//}