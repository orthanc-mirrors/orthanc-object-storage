# README #

Orthanc object-storages plugin for main cloud providers (Google/Azure/AWS)

Check the [Orthanc book](https://book.orthanc-server.com/plugins/object-storage.html) for complete documentation.

## info for developers ##

Here's a sample configuration file of the `StorageEncryption` section of the plugins:

```
{
    "GoogleCloudStorage" : {
        "StorageEncryption" : {
            "Enable": true,
            "MasterKey": [3, "/path/to/master.key"], // key id - path to the base64 encoded key
            "PreviousMasterKeys" : [
                [ 1, "/path/to/previous1.key"],
                [ 2, "/path/to/previous2.key"]
            ],
            "MaxConcurrentInputSize" : 1024   // size in MB 
        }
    }
}
```

### Compile Google plugin ###

* `./vcpkg install google-cloud-cpp`
* `./vcpkg install cryptopp`
* `hg clone ...`
* `mkdir -p build/google`
* `cd build/google` 
* `cmake -DCMAKE_TOOLCHAIN_FILE=[vcpkg root]\scripts\buildsystems\vcpkg.cmake ../../orthanc-object-storage/Google`

### Google plugin configuration ###

```
    "GoogleCloudStorage" : {
        "ServiceAccountFile" : "/.../googleServiceAccountFile.json",
        "BucketName": "test-orthanc-storage-plugin",
        "RootPath": "",                 // optional: folder in which files are stored (ex: my/path/to/myfolder)
        "StorageEncryption" : {...},
        "StorageStructure" : "flat",
        "MigrationFromFileSystemEnabled" : false
    }

```

## Azure Blob Storage plugin ##

### Prerequisites ###

* Install [vcpkg](https://github.com/Microsoft/vcpkg) 

### Compile Azure plugin ###

On Linux:

* `./vcpkg install cpprestsdk`
* `./vcpkg install cryptopp`
* `hg clone ...`
* `mkdir -p build/azure`
* `cd build/azure` 
* `cmake -DCMAKE_TOOLCHAIN_FILE=[vcpkg root]\scripts\buildsystems\vcpkg.cmake ../../orthanc-object-storage/Azure`

On Windows:

* `.\vcpkg.exe install cpprestsdk:x64-windows-static`
* `.\vcpkg.exe install azure-storage-cpp:x64-windows-static`
* `.\vcpkg.exe install cryptopp:x64-windows-static`
* `hg clone ...`
* `mkdir -p build/azure`
* `cd build/azure` 
* `cmake -DCMAKE_TOOLCHAIN_FILE=[vcpkg root]\scripts\buildsystems\vcpkg.cmake -DVCPKG_TARGET_TRIPLET=x64-windows-static -DSTATIC_BUILD=ON -DCMAKE_BUILD_TYPE="Release" ../../orthanc-object-storage/Azure`
* `cmake --build . --config Release`


### Azure plugin configuration ###

```
    "AzureBlobStorage" : {
    	"ConnectionString": "DefaultEndpointsProtocol=https;AccountName=xxxxxxxxx;AccountKey=yyyyyyyy===;EndpointSuffix=core.windows.net",
    	"ContainerName" : "test-orthanc-storage-plugin",
        "CreateContainerIfNotExists": true,       // available from version 1.2.0 (not released yet)
        "RootPath": "",                 // optional: folder in which files are stored (ex: my/path/to/myfolder)
        "StorageEncryption" : {...},
        "StorageStructure" : "flat",
        "MigrationFromFileSystemEnabled" : false
    }
```

## AWS S3 Storage plugin ##

### Prerequisites ###

* Install [vcpkg](https://github.com/Microsoft/vcpkg) 

* compile the AWS C++ SDK

```

mkdir ~/aws
cd ~/aws
git clone https://github.com/aws/aws-sdk-cpp.git

mkdir -p ~/aws/builds/aws-sdk-cpp
cd ~/aws/builds/aws-sdk-cpp
cmake -DBUILD_ONLY="s3;transfer" ~/aws/aws-sdk-cpp 
make -j 4 
make install
```

### Compile AWS S3 plugin ###

* `./vcpkg install cryptopp`
* `hg clone ...`
* `mkdir -p build/aws`
* `cd build/aws` 
* `cmake -DCMAKE_TOOLCHAIN_FILE=[vcpkg root]\scripts\buildsystems\vcpkg.cmake ../../orthanc-object-storage/Aws`

### Azure plugin configuration ###

```
    "AwsS3Storage" : {
    	"BucketName": "test-orthanc-s3-plugin",
        "Region" : "eu-central-1",
        "AccessKey" : "AKXXX",
        "SecretKey" : "RhYYYY",
        "Endpoint": "",                 // optional: custom endpoint
        "ConnectionTimeout": 30,        // optional: connection timeout in seconds
        "RequestTimeout": 1200,         // optional: request timeout in seconds (max time to upload/download a file)
        "RootPath": "",                 // optional: folder in which files are stored (ex: my/path/to/myfolder)
        "StorageEncryption" : {...},    // optional
        "StorageStructure" : "flat",    // optional
        "MigrationFromFileSystemEnabled" : false // optional
    }
```