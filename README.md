# README #

Orthanc object-storages plugin for main cloud providers (Google/Azure/AWS)

## Encryption ##

### Encryption rationale ###

Although all cloud providers already provide encryption at rest, the plugins provide an optional layer of client-side encryption .  It is very important that you understand the scope and benefits of this additional layer of encryption.

Encryption at rest provided by cloud providers basically compares with a file-system disk encryption.  If someone has access to the disk, he won't have access to your data without the encryption key.

With cloud encryption at rest only, if someone has access to the "api-key" of your storage or if one of your admin inadvertently make your storage public, PHI will leak.

Once you use client-side encryption, you'll basically store packets of meaningless bytes on the cloud infrastructure.  So, if an "api-key" leaks or if the storage is misconfigured, packet of bytes will leak but not PHI.

These packets of bytes might eventually not be considered as Personal Health Information (PHI) anymore and eventually help you meet your local regulations (Please check your local regulations).

However, note that, if you're running entirely in a cloud environment, your decryption keys will still be stored on the cloud infrastructure (VM disks - process RAM) and an attacker could still eventually gain access to this keys.  Furthermore, in the scope of the [Cloud Act](https://bitbucket.org/osimis/orthanc-cloud-storages/src/master/UnitTestsSources/EncryptionTests.cpp), the cloud provider might still have the possibility to retrieve your data and encryption key (while it will still be more complex than with standard encryption at rest).

If Orthanc is running in your infrastructure with the Index DB on your infrastructure, and files are store in the cloud, the master keys will remain on your infrastructure only and there's no way the data stored in the cloud could be decrypted outside your infrastructure.


Also note that, although the cloud providers also provide client-side encryption, we, as an open-source project, wanted to provide our own implementation on which you'll have full control and extension capabilities.  This also allows us to implement the same logic on all cloud providers.

Our encryption is based on well-known standards (see below).  Since it is documented and the source code is open-source, feel-free to have your security expert review it before using it in a production environment.

### Encryption technical overview ###

Orthanc saves 2 kind of files: DICOM files and JSON summaries of DICOM files.  Both files contain PHI.

When configuring the plugin, you'll have to provide a `Master Key` that we can also call `Key Encryption Key` (KEK).

For each file being saved, the plugin will generate a new `Data Encryption Key` (DEK).  This DEK, encrypted with the KEK will be pre-pended to the file.

If, at any point, your KEK leaks or you want to rotate your KEKs, you'll be able to use a new one to encrypt new files that are being added and still use the old ones to decrypt data.  You could then eventually start a side script to remove usages of the leaked/obsolete KEKs.

To summarize:

- We use (Crypto++)[https://www.cryptopp.com/] to perform all encryptions.  
- All keys (KEK and DEK) are AES-256 keys.
- DEKs and IVs are encrypted by KEK using CTR block cipher using a null IV.
- data is encrypted by DEK using GCM block cipher that will also perform integrity check on the whole file.

The format of data stored on disk is therefore the following:
- `VERSION HEADER`: 2 bytes: identify the structure of the following data
- `MASTER KEY ID`: 4 bytes: a numerical ID of the KEK that was used to encrypt the DEK
- `EIV`: 32 bytes: IV used by DEK for data encryption; encrypted by KEK
- `EDEK`: 32 bytes: the DEK encrypted by the KEK.
- `CIPHER TEXT`: variable length: the DICOM/JSON file encrypted by the DEK
- `TAG`: 16 bytes: integrity check performed on the whole encrypted file (including header, master key id, EIV and EDEK)

### Configuration ###

AES Keys shall be 32 bytes long (256 bits) and encoded in base64.  Here's a sample OpenSSL command to generate such a key:

```
openssl rand -base64 -out /tmp/test.key 32
```

Each key must have a unique id that is a uint32 number.

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

*MaxConcurrentInputSize*: Since the memory used during encryption/decryption can grow up to a bit more than 2 times the input, we want to limit the number of threads doing concurrent processing according to the available memory instead of the number of concurrent threads.  Therefore, if you're currently
ingesting small files, you can have a lot of thread working together while, if you're ingesting large files, threads might have to wait before receiving a "slot" to access the encryption module.


## Google Cloud Storage plugin ##

### Prerequisites ###

* Install [vcpkg](https://github.com/Microsoft/vcpkg) 

### Compile Google plugin ###

* `./vcpkg install google-cloud-cpp`
* `./vcpkg install cryptopp`
* `hg clone ...`
* `mkdir -p build/google`
* `cd build/google` 
* `cmake -DCMAKE_TOOLCHAIN_FILE=[vcpkg root]\scripts\buildsystems\vcpkg.cmake ../../orthanc-cloud-storages/google`

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
* `cmake -DCMAKE_TOOLCHAIN_FILE=[vcpkg root]\scripts\buildsystems\vcpkg.cmake ../../orthanc-cloud-storages/Azure`

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
* `cmake -DCMAKE_TOOLCHAIN_FILE=[vcpkg root]\scripts\buildsystems\vcpkg.cmake ../../orthanc-cloud-storages/Aws`

### Azure plugin configuration ###

```
    "AwsS3Storage" : {
    	"BucketName": "test-orthanc-s3-plugin",
        "Region" : "eu-central-1",
        "AccessKey" : "AKXXX",
        "SecretKey" : "RhYYYY"
    }
```
