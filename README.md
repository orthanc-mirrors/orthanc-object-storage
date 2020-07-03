# README #

Orthanc object-storages plugin for main cloud providers (Google/Azure/AWS)

Check the [Orthanc book](https://book.orthanc-server.com/plugins/object-storage.html) for complete documentation.

## info for developers ##

Here's a sample configuration file of the `StorageEncryption` section of the plugins:

```
{
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
```

### Compile Google plugin ###

* `./vcpkg install google-cloud-cpp`
* `./vcpkg install cryptopp`
* `hg clone ...`
* `mkdir -p build/google`
* `cd build/google` 
* `cmake -DCMAKE_TOOLCHAIN_FILE=[vcpkg root]\scripts\buildsystems\vcpkg.cmake ../../orthanc-object-storage/google`

### Google plugin configuration ###

```
    "GoogleCloudStorage" : {
        "ServiceAccountFile": "/.../googleServiceAccountFile.json",
        "BucketName": "test-orthanc-storage-plugin"
    }

```

## Azure Blob Storage plugin ##

### Prerequisites ###

* Install [vcpkg](https://github.com/Microsoft/vcpkg) 

### Compile Azure plugin ###

* `./vcpkg install cpprestsdk`
* `hg clone ...`
* `mkdir -p build/azure`
* `cd build/azure` 
* `cmake -DCMAKE_TOOLCHAIN_FILE=[vcpkg root]\scripts\buildsystems\vcpkg.cmake ../../orthanc-object-storage/Azure`

### Azure plugin configuration ###

```
    "AzureBlobStorage" : {
    	"ConnectionString": "DefaultEndpointsProtocol=https;AccountName=xxxxxxxxx;AccountKey=yyyyyyyy===;EndpointSuffix=core.windows.net",
    	"ContainerName" : "test-orthanc-storage-plugin"
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
        "SecretKey" : "RhYYYY"
    }
```